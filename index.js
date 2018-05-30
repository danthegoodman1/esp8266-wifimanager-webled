const express = require("express");
const bodyParser = require("body-parser");
const cors = require("cors");

const app = express();
app.use(bodyParser.json());

let redC = 255;
let blueC = 255;
let greenC = 255;

app.options("*", cors());
app.get("/", cors(), (req, res) => {
    console.log("request!!!!!");
    res.send("Hello");
})
.get("/getcolors", (req, res) => {
    let colors = {
        red: redC,
        blue: blueC,
        green: greenC
    };

    res.send(JSON.stringify(colors))
})
.post("/setcolors", cors(), (req, res) => {
    redC = req.body.redColor;
    console.log(`Set Red to: ${redC}`);
    blueC = req.body.blueColor;
    console.log(`Set Blue to: ${blueC}`);
    greenC = req.body.greenColor;
    console.log(`Set Green to: ${greenC}`);
    res.send("Got colors");
});

app.listen(80, "0.0.0.0", () => {
    console.log("listening!");
    setInterval(function(){
        console.log(`Current Colors: Red: ${redC}, Green: ${greenC}, Blue: ${blueC}`);
    }, 2000);
});
