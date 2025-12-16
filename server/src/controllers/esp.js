const pool = require('../models/db');
const Esp = require('../models/esp');

const esp = new Esp(pool);

const sensors = [
    {name:"temp",val:[200,1000,555,345]},{},{}
];

const createAVGsensors = async (req,res) => {
    try{
        const {name,val,id_pot}=req.body; // שומר את הבדיקות בבסיס הנתונים. אם היינו שומרים בשרת והשרת היה נופל אז היינו מאבדים את הבדיקות
        if((name!="temp" && val<=0) || (name=="" || id_pot<0)){
            return res.status(401).json({message:"one or more of the parameters are missing or wrong."});
        }
        let data = await esp.createAvgSensorQu(name,val,id_pot);
        console.log(data);
        return(res.status(201).json({message:"the check has been saved successfully."}));
    } catch (error){
        console.log(error);
    }
}

const readAvgDate = async (req,res) => {
    try{
        const {name,date}=req.body;
        //SELECT AVG(Price)
        let data = await esp.readAvgDateQu(name,date);
        console.log(data);
        return(res.status(201).json(data));
    } catch (error){
        console.log(error);
    }
}

const readAvgPot = async (req,res) => {
    /*try{
        const {name,val,id_pot}=req.body; // שומר את הבדיקות בבסיס הנתונים. אם היינו שומרים בשרת והשרת היה נופל אז היינו מאבדים את הבדיקות
        if((name!="temp" && val<=0) || (name=="" || id_pot<0)){
            return res.status(401).json({message:"one or more of the parameters are missing or wrong."});
        }
        let data = await esp.createAvgSensorQu(name,val,id_pot);
        console.log(data);
        return(res.status(201).json({message:"the check has been saved successfully."}));
    } catch (error){
        console.log(error);
    }*/
}

const deleteAvgSensor = async (req,res) => {
    /*try{
        const {name,val,id_pot}=req.body; // שומר את הבדיקות בבסיס הנתונים. אם היינו שומרים בשרת והשרת היה נופל אז היינו מאבדים את הבדיקות
        if((name!="temp" && val<=0) || (name=="" || id_pot<0)){
            return res.status(401).json({message:"one or more of the parameters are missing or wrong."});
        }
        let data = await esp.createAvgSensorQu(name,val,id_pot);
        console.log(data);
        return(res.status(201).json({message:"the check has been saved successfully."}));
    } catch (error){
        console.log(error);
    }*/
}

module.exports = {createAVGsensors,readAvgDate,readAvgPot,deleteAvgSensor,}