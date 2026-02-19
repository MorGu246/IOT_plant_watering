const pool = require('../models/db');
const Esp = require('../models/esp');
const mqtt = require('mqtt');

const esp = new Esp(pool);

// התחברות ל-Broker (המחשב שלך או שרת מרוחק)
const mqttClient = mqtt.connect('mqtt://10.0.0.5'); //10.9.25.238

mqttClient.on('connect', () => {
    console.log("Connected to MQTT Broker");
});

// --- פונקציה חדשה לשליחת פקודות ל-ESP32 ---
// מטפל בנתיב: POST /esp/change-mode
const updatePotMode = async (req, res) => {
    try {
        const { potId, mode } = req.body;
        if (!potId || mode === undefined) {
            return res.status(400).json({ message: "potId and mode are required" });
        }
        // שליחת ההודעה לארדואינו דרך MQTT
        const topic = `pot/${potId}/mode`;
        mqttClient.publish(topic, mode.toString(), { qos: 1 }, (err) => {
            if (err) {
                console.error("MQTT Publish Error:", err);
                return res.status(500).json({ message: "Failed to send MQTT message" });
            }
            console.log(`Sent mode ${mode} to pot ${potId} via MQTT`);
            return res.status(200).json({ message: `Mode changed to ${mode}` });
        });
    } catch (error) {
        console.error("Error in updatePotMode:", error);
        res.status(500).json({ message: "Server error" });
    }
};

// מטפל בנתיב: POST /esp/send-command
const sendCommand = async (req, res) => {
    try {
        const { potId, command } = req.body; // command יהיה "START" או "STOP"
        if (!potId || !command) {
            return res.status(400).json({ message: "potId and command are required" });
        }
        const topic = `pot/${potId}/command`;
        // שליחת ההודעה ב-MQTT
        mqttClient.publish(topic, command, { qos: 1 }, (err) => {
            if (err) {
                console.error("MQTT Publish Error (Command):", err);
                return res.status(500).json({ message: "Failed to send command via MQTT" });
            }
            console.log(`Sent manual command ${command} to pot ${potId}`);
            return res.status(200).json({ message: `Command ${command} sent successfully` });
        });
    } catch (error) {
        console.error("Error in sendCommand:", error);
        res.status(500).json({ message: "Server error" });
    }
};

// const sensors = [
//     {name:"temp",val:[200,1000,555,345]},{},{}
// ];
function sendJsonToServer(name, avg, potId) {
  console.log({
    name,
    avg,
    potId
  });
}

sendJsonToServer("temp", 25.5, 1001);
//  * 1. קבלת נתונים מה-ESP32 ושמירתם
//  * מטפל בנתיב: POST /esp/create
const createAVGsensors = async (req, res) => {
    try {
        const { name, avg, potId } = req.body;
        // בדיקת תקינות בסיסית
        if (!name || avg === undefined || !potId) {
            return res.status(400).json({ message: "Missing parameters: name, avg, or potId" });
        }
        // --- ניהול שלמות הנתונים (Foreign Keys) ---
        // א. בדיקה אם העציץ (Pot) קיים
        const [pots] = await pool.execute("SELECT * FROM pots WHERE id = ?", [potId]);
        if (pots.length === 0) {
            console.log(`Pot ${potId} not found. Creating it now...`);
            // ב. בדיקה אם קיים סוג צמח ברירת מחדל (Species id=1)
            const [species] = await pool.execute("SELECT * FROM species WHERE id = ?", [1]);
            if (species.length === 0) {
                await pool.execute("INSERT INTO species (id, type, list) VALUES (1, 'General', 'Default species')");
            }
            // ג. יצירת העציץ החדש
            await pool.execute(
                "INSERT INTO pots (id, type_id, date, name, status) VALUES (?, 1, NOW(), ?, 'active')",
                [potId, `Pot ${potId}`]
            );
        }
        // --- שמירת הנתון בטבלת sensors ---
        // משתמשים בפונקציה מה-Model החדש
        await esp.createSensorReading(name, avg, potId);
        console.log(`Saved reading: ${name} = ${avg} for Pot ${potId}`);
        return res.status(201).json({ message: "Data saved successfully" });
    } catch (error) {
        console.error("Error in createAVGsensors:", error);
        return res.status(500).json({ message: "Internal Server Error" });
    }
};
//  * 2. שליפת ממוצעים עבור הדשבורד (גרפים)
//  * מטפל בנתיב: POST /esp/analytics
const readAvgDate2 = async (req, res) => {
    try {
        const { name, date } = req.body; // מצפה לפורמט YYYY-MM-DD
        if (!name || !date) {
            return res.status(400).json({ message: "Name and date are required" });
        }
        const result = await esp.getAvgByNameAndDate(name, date);
        if (!result || result.total_samples === 0) {
            return res.status(404).json({ message: "No data found for this date" });
        }
        return res.status(200).json({
            sensor: name,
            date: date,
            average: Number(result.avg_value.toFixed(2)),
            count: result.total_samples
        });
    } catch (error) {
        console.error("Error in readAvgDate2:", error);
        return res.status(500).json({ message: "Server error" });
    }
};
//  * 3. מחיקת עציץ וכל נתוניו (Maintenance)
const deletePot = async (req, res) => {
    try {
        const { id_pot } = req.params; // עדיף לקבל כפרמטר ב-URL
        // קודם מוחקים את החיישנים (בגלל ה-Foreign Key)
        await esp.deleteSensorsByPot(id_pot);
        // עכשיו מוחקים את העציץ עצמו
        const [result] = await pool.execute("DELETE FROM pots WHERE id = ?", [id_pot]);
        if (result.affectedRows === 0) {
            return res.status(404).json({ message: "Pot not found" });
        }
        return res.status(200).json({ message: "Pot and its sensors deleted successfully" });
    } catch (error) {
        console.error("Error in deletePot:", error);
        return res.status(500).json({ message: "Server error" });
    }
};
//  * 4. תיעוד השקיה שהתבצעה בפועל
//  * נקרא מה-ESP32 ברגע שהמנוע עוצר (stopMotor)
//  * POST /esp/water-log
const logIrrigation = async (req, res) => {
    try {
        const { potId, duration } = req.body; // duration זה ה-'count' בטבלה שלך
        if (!potId || duration === undefined) {
            return res.status(400).json({ message: "potId and duration are required" });
        }
        await esp.createWaterLog(potId, duration);
        console.log(`Irrigation logged for Pot ${potId}: ${duration} minutes`);
        return res.status(201).json({ message: "Water log saved" });
    } catch (error) {
        console.error("Error in logIrrigation:", error);
        return res.status(500).json({ message: "Server error" });
    }
};
//  * 5. קבלת פרטים מלאים על עציץ (כולל סוג הצמח)
//  * מיועד להצגה בדשבורד (Front-end)
//  * GET /esp/pot-details/:id
const getPotDetails = async (req, res) => {
    try {
        const { id } = req.params;
        const details = await esp.getPotWithSpecies(id);
        if (!details) {
            return res.status(404).json({ message: "Pot not found" });
        }
        return res.status(200).json(details);
    } catch (error) {
        console.error("Error in getPotDetails:", error);
        return res.status(500).json({ message: "Server error" });
    }
};
// פונקציית בדיקה עצמית שמכניסה נתון ל-DB ברגע שהשרת עולה
const testDatabaseConnection = async () => {
    try {
        console.log("--- Running DB Test ---");
        // 1. הכנסת נתונים (כבר עשית)
        await esp.createSensorReading("temp", 20.0, 1001);
        await esp.createSensorReading("temp", 24.0, 1001);
        await esp.createSensorReading("temp", 26.0, 1001);
        await esp.createSensorReading("temp", 30.0, 1001);
        // 2. חישוב התאריך של היום בפורמט YYYY-MM-DD
        const today = new Date().toISOString().slice(0, 10); 
        // 3. קריאה ישירה ל-MODEL (ולא ל-Controller!)
        const result = await esp.getAvgByNameAndDate("temp", today);
        console.log("--- Test Results ---");
        console.log(`Sensor: temp, Date: ${today}`);
        console.log(`Average found: ${result.avg_value}`); // אמור להיות 25
        console.log(`Total samples: ${result.total_samples}`); // אמור להיות 4
        console.log("Test successful: Data inserted and averaged!");
    } catch (err) {
        console.error("Test failed: Could not connect to MySQL", err.message);
    }
};

// הפעלת הבדיקה (תוריד את זה אחרי שראית שזה עובד)
//testDatabaseConnection();
module.exports = {createAVGsensors, readAvgDate2, deletePot, logIrrigation, getPotDetails, updatePotMode, sendCommand};