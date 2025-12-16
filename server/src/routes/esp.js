const router = require("express").Router();

const {createAVGsensors,readAvgDate,readAvgPot,deleteAvgSensor} = require('../controllers/esp').Router();

router.post('/create',createAVGsensors);
router.get('/read',readAvgDate);
router.get('/pot',readAvgPot);
router.delete('/delete',deleteAvgSensor);

module.exports = router;