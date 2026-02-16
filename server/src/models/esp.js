class Esp {
    constructor(db) {
        this.db = db;
    }
    /**
     * 1. שמירת נתוני חיישן (טמפרטורה, אור, לחות)
     * טבלה: sensors
     */
    async createSensorReading(name, avg, potId) {
        // שימוש בפורמט YYYY-MM-DD עבור עמודת ה-date במסד הנתונים
        const date = new Date().toISOString().slice(0, 10);
        let sql = "INSERT INTO sensors (sensor_name, val_avg, date, pot_id) VALUES (?, ?, ?, ?);";
        return await this.db.execute(sql, [name, avg, date, potId]);
    }
    /**
     * 2. תיעוד השקיה (לוג עבודה של המנוע)
     * טבלה: water_follow
     * count = משך ההשקיה או כמות
     */
    async createWaterLog(potId, count) {
        const now = new Date();
        const date = now.toISOString().slice(0, 10); // YYYY-MM-DD
        const time = now.toTimeString().slice(0, 8); // HH:MM:SS
        let sql = "INSERT INTO water_follow (date, time, count, pot) VALUES (?, ?, ?, ?);";
        return await this.db.execute(sql, [date, time, count, potId]);
    }
    /**
     * 3. קבלת פרטי עציץ כולל סוג הצמח שלו (JOIN בין pots ל-species)
     * יחס: species (1) -> pots (N)
     */
    async getPotWithSpecies(potId) {
        let sql = `
            SELECT pots.name, pots.status, species.type, species.list 
            FROM pots 
            JOIN species ON pots.type_id = species.id 
            WHERE pots.id = ?;
        `;
        const [rows] = await this.db.execute(sql, [potId]);
        return rows[0];
    }
    /**
     * 4. קבלת ממוצע נתונים לפי שם חיישן ותאריך
     * טבלה: sensors
     */
    async getAvgByNameAndDate(name, date) {
        // שימוש ב-DATE() במידה והעמודה היא DateTime, או השוואה ישירה אם היא Date
        const sql = "SELECT AVG(val_avg) AS avg_value, COUNT(*) AS total_samples FROM sensors WHERE sensor_name = ? AND date = ?";
        const [rows] = await this.db.execute(sql, [name, date]);
        return rows[0];
    }
    /**
     * 5. מחיקת נתוני חיישנים של עציץ ספציפי
     */
    async deleteSensorsByPot(potId) {
        let sql = "DELETE FROM sensors WHERE pot_id = ?;";
        return await this.db.execute(sql, [potId]);
    }
}
module.exports = Esp;