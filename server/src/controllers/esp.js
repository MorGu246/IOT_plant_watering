const pool = require('../models/db');
const Esp = require('../models/esp');

const esp = new Esp(pool);

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
//sendJsonToServer("temp", 25.5, 1001);

// const createAVGsensors = async (req,res) => {
//     try{
//         const {name,avg,potId}=req.body; // שומר את הבדיקות בבסיס הנתונים. אם היינו שומרים בשרת והשרת היה נופל אז היינו מאבדים את הבדיקות
//         if((name!="temp" && avg<=0) || (name=="" || potId<0)){
//             return res.status(401).json({message:"one or more of the parameters are missing or wrong."});
//         }
//         //////////////////////////////
//         const [pots] = await pool.execute("SELECT * FROM pots WHERE id = ?", [potId]);
//         if (pots.length === 0) {
//         // אם Pot לא קיים, מוסיפים אותו
//             //await pool.execute("INSERT INTO pots (id, name) VALUES (?, ?)", [potId, `Pot ${potId}`]);//pot_name
//                     await pool.execute(
//                     `INSERT INTO pots (id, type_id, date, name, status)
//                     VALUES (?, ?, NOW(), ?, ?)`,
//                     [potId, 1, `Pot ${potId}`, "active"] // type_id = 1 חייב להיות קיים בטבלת species
//                     );
//             console.log(`Added new pot with id ${potId}`);
//         }
//         //////////////////////////////
//         let data = await esp.createAvgSensorQu(name,avg,potId);
//         console.log(data);
//         console.log(req.body);
//         return(res.status(201).json({message:"the check has been saved successfully."}));
//     } catch (error){
//         console.log(error);
//         return res.status(500).json({ message: "Server error" });//////////
//     }
// }

const createAVGsensors = async (req,res) => {
    try {
        const {name, avg, potId} = req.body;
        
        // בדיקה בסיסית של הקלט
        if ((name != "temp" && avg <= 0) || (name == "" || potId < 0)) {
            return res.status(401).json({message: "one or more of the parameters are missing or wrong."});
        }

        //////////////////////////////
        // בודק אם ה-pot כבר קיים
        const [pots] = await pool.execute("SELECT * FROM pots WHERE id = ?", [potId]);
        if (pots.length === 0) {

            // **בדיקה אם species עם id=1 קיים, אם לא מוסיפים אותו**
            const [species] = await pool.execute("SELECT * FROM species WHERE id = ?", [1]);
            if (species.length === 0) {
                await pool.execute(
                    "INSERT INTO species (id, type, list) VALUES (?, ?, ?)",
                    [1, 'Default Species', '']
                );
                console.log("Added default species with id 1");
            }

            // עכשיו אפשר להכניס את ה-pot החדש בלי לקבל שגיאת Foreign Key
            await pool.execute(
                `INSERT INTO pots (id, type_id, date, name, status)
                 VALUES (?, ?, NOW(), ?, ?)`,

                [potId, 1, `Pot ${potId}`, "active"]
            );
            console.log(`Added new pot with id ${potId}`);
        }

        //////////////////////////////
        // שורה ששומרת את החיישן
        let data = await esp.createAvgSensorQu(name, avg, potId);
        console.log(data);
        console.log(req.body);

        return res.status(201).json({message:"the check has been saved successfully."});

    } catch (error) {
        console.log(error);
        return res.status(500).json({ message: "Server error" });
    }
};

const readAvgDate = async (req,res) => {
    try{
        const {name,date}=req.body;
        //SELECT AVG(Price)
        let data = await esp.readAvgDateQu(name,date); // לא טוב כי אמור להיות רק date בלי name
        console.log(data);
        return(res.status(201).json(data));
    } catch (error){
        console.log(error);
    }
}

const readAvgDate2 = async (req, res) => {
    try {
        const { name, date } = req.body;
        if (!name || !date) {
            return res.status(400).json({
                message: "name and date are required"
            });
        }
        const result = await esp.readAvgByNameAndDate(name, date);
        if (!result || result.total_samples === 0) {
            return res.status(404).json({
                message: "No samples found for this date"
            });
        }
        return res.status(200).json({
            name,
            date,
            avg: Number(result.avg_value.toFixed(2)),
            samples: result.total_samples
        });
    } catch (error) {
        console.error(error);
        res.status(500).json({ message: "Server error" });
    }
};

const readAvgPot = async (req,res) => {
    try{
        const {name,val,id_pot}=req.body; // שומר את הבדיקות בבסיס הנתונים. אם היינו שומרים בשרת והשרת היה נופל אז היינו מאבדים את הבדיקות
        if((name!="temp" && val<=0) || (name=="" || id_pot<0)){
            return res.status(401).json({message:"one or more of the parameters are missing or wrong."});
        }
        let data = await esp.readAvgPotQu(id_pot);
        console.log(data);
        return(res.status(201).json({message:"the check has been saved successfully."}));
    } catch (error){
        console.log(error);
    }
}

const deleteAvgSensor = async (req,res) => {
    try{
        const {id_pot}=req.body; // שומר את הבדיקות בבסיס הנתונים. אם היינו שומרים בשרת והשרת היה נופל אז היינו מאבדים את הבדיקות
        if(!id_pot || id_pot <= 0){
            return res.status(400).json({message:"pot does not exist"});
        }
        /*let data = await esp.deleteAvgPotQu(id_pot);
        if (data.affectedRows === 0) {
            return res.status(404).json({ message: "pot not found" });
        }*/
        let [result] = await esp.deleteAvgPotQu(id_pot);

        if (result.affectedRows === 0) {
            return res.status(404).json({ message: "pot not found" });
        }
        console.log(data);
        return(res.status(200).json({message:"the pot has been removes successfully."}));
    } catch (error){
        console.log(error);
        return res.status(500).json({ message: "Server error" });
    }
};
//////////////////////////////////////////////////////////////////////
// async function sendJsonToServer(name, avg, potId) {
//   const payload = { name, avg, potId };
//   console.log("Sending JSON:");
//   console.log(payload);
// }
//////////////////////////////////////////////////////////////////////
module.exports = {createAVGsensors,readAvgDate2,readAvgPot,deleteAvgSensor,}//sendJsonToServer,