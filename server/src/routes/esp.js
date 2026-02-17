const router = require("express").Router();

const {createAVGsensors,readAvgDate2,deletePot,logIrrigation, getPotDetails,updatePotMode/*readAvgPot,deleteAvgSensor*/} = require('../controllers/esp');

router.post('/create',createAVGsensors);
router.get('/analytics',readAvgDate2);
// router.get('/pot',readAvgPot);
// router.delete('/delete',deleteAvgSensor);
router.delete('/delete',deletePot);
router.get('/water-log',logIrrigation);
router.get('/details',getPotDetails);
router.post('/change-mode', updatePotMode);

module.exports = router;