// שם מלא: מור גואטה ת.ז.: 314813379
const router = require("express").Router();

const {createAVGsensors,readAvgDate2,deletePot,logIrrigation, getPotDetails,updatePotMode, sendCommand, getWeeklyAnalytics} = require('../controllers/esp');

router.post('/create',createAVGsensors);
router.get('/analytics',readAvgDate2);
router.delete('/delete',deletePot);
router.get('/water-log',logIrrigation);
router.get('/details',getPotDetails);
router.post('/change-mode', updatePotMode);
router.post('/send-command', sendCommand);
router.get('/analytics/:potId', getWeeklyAnalytics)

module.exports = router;