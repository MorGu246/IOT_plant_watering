const pool = require('../models/db');
const Esp = require('../models/esp');

const esp = new Esp(pool);

const sensors = [
    {namee:"טמפ'",val:[200,1000,555,345]},{},{}
];

const createAVGsensors = async (req,res) => {
    try{
        const data = req.body;
        for(let k = 0; k < sensors.length + 1; k++){
            let item = {};
            for(let i = 0; i < data.length; i++){
                item = {name: data[i].name,
                        id_pot: data[i].id_pot,
                        val:[...sensors[k].val, data[i].val]
                }
            }
            sensors[k] = item;
        }
    } catch (error){

    }
}