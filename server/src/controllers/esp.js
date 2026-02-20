// שם מלא: מור גואטה ת.ז.: 314813379
const pool = require('../models/db');
const Esp = require('../models/esp');
const mqtt = require('mqtt');

const esp = new Esp(pool);

const mqttClient = mqtt.connect('mqtt://10.0.0.5');

mqttClient.on('connect', () => {
    console.log("Connected to MQTT Broker");
});

// --- פונקציה חדשה לשליחת פקודות ל-ESP32 ---
const updatePotMode = async (req, res) => {
    try {
        const { potId, mode } = req.body;
        if (!potId || mode === undefined) {
            return res.status(400).json({ message: "potId and mode are required" });
        }
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

const sendCommand = async (req, res) => {
    try {
        const { potId, command } = req.body;
        if (!potId || !command) {
            return res.status(400).json({ message: "potId and command are required" });
        }
        const topic = `pot/${potId}/command`;
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

//  * 1. קבלת נתונים מה-ESP32 ושמירתם
const createAVGsensors = async (req, res) => {
    try {
        const { name, avg, potId } = req.body;
        if (!name || avg === undefined || !potId) {
            return res.status(400).json({ message: "Missing parameters: name, avg, or potId" });
        }
        const [pots] = await pool.execute("SELECT * FROM pots WHERE id = ?", [potId]);
        if (pots.length === 0) {
            console.log(`Pot ${potId} not found. Creating it now...`);
            const [species] = await pool.execute("SELECT * FROM species WHERE id = ?", [1]);
            if (species.length === 0) {
                await pool.execute("INSERT INTO species (id, type, list) VALUES (1, 'General', 'Default species')");
            }
            await pool.execute(
                "INSERT INTO pots (id, type_id, date, name, status) VALUES (?, 1, NOW(), ?, 'active')",
                [potId, `Pot ${potId}`]
            );
        }
        await esp.createSensorReading(name, avg, potId);
        console.log(`Saved reading: ${name} = ${avg} for Pot ${potId}`);
        return res.status(201).json({ message: "Data saved successfully" });
    } catch (error) {
        console.error("Error in createAVGsensors:", error);
        return res.status(500).json({ message: "Internal Server Error" });
    }
};
//  * 2. שליפת ממוצעים עבור הדשבורד (גרפים)
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
        const { id_pot } = req.params;
        await esp.deleteSensorsByPot(id_pot);
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
const logIrrigation = async (req, res) => {
    try {
        const { potId, duration } = req.body;
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

const getWeeklyAnalytics = async (req, res) => {
    try {
        const { potId } = req.params;
        const sensorData = await esp.getWeeklySensorStats('temp', potId);
        const waterData = await esp.getWeeklyWaterStats(potId);
        return res.status(200).json({
            sensors: sensorData,
            water: waterData
        });
    } catch (error) {
        console.error("Error in getWeeklyAnalytics:", error);
        return res.status(500).json({ message: "Server error" });
    }
};

module.exports = {createAVGsensors, readAvgDate2, deletePot, logIrrigation, getPotDetails, updatePotMode, sendCommand, getWeeklyAnalytics};