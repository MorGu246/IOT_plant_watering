// שם מלא: מור גואטה ת.ז.: 314813379
require("dotenv").config();
const express = require('express');
const morgan = require('morgan');
const cors = require('cors');
const path = require('path');
const routes = require("./routes")

const port = process.env.port || 3671;
const app = express();
app.use(express.static(path.join(__dirname,"../public"))); // סטטיק זה שנוכל לגשת לתיקייה או קובץ מכל מקום בקוד מבלי לנתב אליו
app.use(express.json());

app.use(morgan("dev"));
app.use(cors());
app.use("/",routes);


app.listen(port, '0.0.0.0', () => {
    console.log(`The server is running on port${port}....`);
});