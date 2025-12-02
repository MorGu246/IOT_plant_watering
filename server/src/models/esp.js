class Esp{
    constructor(db){
        this.db = db;
    }
    async createAvgSensor(name, avg, potId){
        let sql = "INSERT INTO sensors (sensor_name, val_avg, date, pot_id) VALUES (?,?,?,?);"//הגנה מאינג'קשן בעזרת סימני שאלה
        return await this.db.execute(sql,[name,avg,new Date().toLocaleDateString("he-IL"),potId]);
    }
        async readAvgSensor(name){
        let sql = "SELECT * FROM sensors WHERE sensor_name = ?;";
        return await this.db.execute(sql, [name]);
    }
        async readAvgPot(potId){
        let sql = "SELECT * FROM sensors WHERE  pot_id = ?;";
        return await this.db.execute(sql, [potId]);
    }
    async deleteAvgSensor(id){
        let sql = "DELETE FROM sensors WHERE sensor_id = ?;";
        return await this.db.execute(sql, [id]);
    }
}

module.exports = Esp;