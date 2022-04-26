
const express = require("express");
const path = require("path");
const bodyParser = require('body-parser');
const mqtt = require('mqtt');


const app = express();
const port = process.env.PORT || "3000";

const client = mqtt.connect("mqtt://MQTT ip address:port");
const topic = "HP/+";

var moisture ="";
var automatic="";
var limit = 0;
var urlencodedParser = bodyParser.urlencoded({extended: false});

client.on("connect", () => {
  console.log('connected to mqtt');
  client.subscribe((topic), () => {
    console.log(`Subscribed to topic ${topic}`);
  });
});


client.on('message', function(topic, payload){
    if (topic === "HP/moisture"){
      console.log('Received Message: ', topic, payload.toString());
      moisture = payload;
    }
    if (topic === "HP/mode"){
      console.log("Received message: ", topic, payload.toString());
      automatic = payload;
    }
});
 
app.set("views", path.join(__dirname, "views"));
app.set("view engine", "pug");

app.set("views", path.join(__dirname, "views"));
app.set("view engine", "pug");
app.use(express.static(path.join(__dirname, "public")));

app.get("/", (req, res) => {
  if (automatic == "ON"){
    res.render('index', {value: moisture.toString(), mode: `Current mode: ${automatic} with limit: ${limit}%` });
  }
  else{
    res.render('index', {value: moisture.toString(), mode: `Current mode: ${automatic}` });
  }
});

app.get("/water", (req, res) => {
  if (automatic == "ON"){
    res.render('index', {message: 'Plant says: Thanks!', value: moisture.toString(), mode: `Current mode: ${automatic} with limit: ${limit}%`});
    console.log("Watering called!");
    client.publish("HP/water", "water");
  }
  else {
    res.render('index', {message: 'Plant says: Thanks!', value: moisture.toString(), mode: `Current mode: ${automatic}`});
    console.log("Watering called!");
    client.publish("HP/water", "water");
  }
});

app.post("/auto", urlencodedParser, (req, res) => { //Only positive limit values are accepted.
    limit = Number(req.body.limits);
    if(limit > 0 && limit <= 100) {
      automatic="ON";
      console.log(`Automatic ${automatic}, Moisture limit is ${limit}`);
      res.render('index', {value: moisture.toString(), mode: `Current mode: ${automatic} with limit: ${limit}%` });
      client.publish("HP/auto", req.body.limits);
    }
    else {
      res.render('index', {value: moisture.toString(), mode: `Current mode: ${automatic}`});
    }
});

app.post("/auto/stop", (req, res) => {
  automatic = "OFF";
  console.log(`Automatic ${automatic}`);
  client.publish("HP/auto/stop", automatic);
  res.render('index', {value: moisture.toString(), mode: `Current mode: ${automatic}`});
});

app.listen(port, () => {
  console.log(`Listening to requests on http://localhost:${port}`);
});
