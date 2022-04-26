// MoistureSensor pins
#define sensorPower 25
#define sensorPin A0
#define pumpLed 33
#define autoLed 32
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <string.h>

const int relay = 26;
char moistString[8];
long lastMsg = 0;

char* automatic = "OFF"; //Global variables needed for the automatic mode.
int limit = NULL;
float curMoist = NULL;

// Wifi setup and mqtt broker
const char* ssid = "Your wifi ssid";
const char* password = "wifi password";
const char* mqtt_server = "mqtt ip address";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  //Connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//This function receives messages from the MQTT
void callback(char* topic, byte* message, unsigned int length) {
  //Message and topic are printed to the serial monitor to verify the values.
  String topicTemp = String(topic);
  Serial.println("Callback function");
  Serial.print("Message arrived on topic: ");
  Serial.print(topicTemp);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  
  Serial.println();
  
  if(topicTemp == "HP/water"){
    pumpOn();
  }
  else if(topicTemp == "HP/auto") {
    limit = messageTemp.toInt();
    automatic = "ON";
    digitalWrite(autoLed, HIGH);
    client.publish("HP/mode", automatic);
    Serial.print("Automatic on with limit: ");
    Serial.print(limit);
    Serial.println("%");
    }
  else if(topicTemp == "HP/auto/stop"){
    automatic = "OFF";
    digitalWrite(autoLed, LOW);
    client.publish("HP/mode", automatic);
    Serial.println("Automatic OFF");
    }
 
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("HP/auto");
      client.subscribe("HP/water");
      client.subscribe("HP/auto/stop");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.println("Void setup");
  pinMode(sensorPower, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(pumpLed, OUTPUT);
  pinMode(autoLed, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1884);
  client.setCallback(callback);
   
  // Initially keep the sensor OFF
  digitalWrite(sensorPower, LOW);
  
  Serial.begin(9600);
}

//  This function reads the moisture level and publishes it.
int readSensor() {
  Serial.println("Read sensor function");
  digitalWrite(sensorPower, HIGH);  // Turn the sensor ON
  delay(10);              // Allow power to settle
  int val = analogRead(sensorPin);  // Read the analog value form sensor
  digitalWrite(sensorPower, LOW);   // Turn the sensor OFF
    
  Serial.print("Analog output: ");
  Serial.println(val);
  //To percentage
  float percentage = map(val, 0, 2300, 0, 100);

  Serial.print("Moisture percentage: ");
  Serial.print(percentage);

  dtostrf(percentage, 1, 2, moistString); //Changed from float to string.

  
  Serial.println("%");
  client.publish("HP/moisture", moistString);
  return percentage;
}

void pumpOn(){
  Serial.println("PumpOn function");
  digitalWrite(pumpLed, HIGH);
  delay(1000);
  digitalWrite(relay,HIGH);
  Serial.println("Pump ON");
  delay(1000);
  digitalWrite(relay, LOW);
  Serial.println("Pump OFF");
  digitalWrite(pumpLed, LOW);
  }


void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (curMoist == NULL){
    curMoist = readSensor();
    client.publish("HP/mode", automatic);
    client.publish("HP/limit", limit);
  }
  
  if (automatic == "ON"){
    if (curMoist < limit && curMoist > 0) {
      Serial.println("Watering in automatic mode");
      pumpOn();
      delay(2000);
      curMoist = readSensor();
      }
    }
  
  long now = millis();  
  if (now - lastMsg > 10000) {
    lastMsg = now;
    curMoist = readSensor();
    Serial.print("Current moisture is: ");
    Serial.println(curMoist);

    Serial.print("Automatic mode is: ");
    Serial.print(automatic);
    Serial.print(" with limit value of: ");
    Serial.println(limit);
    client.publish("HP/mode", automatic);
    client.publish("HP/limit", limit);
  }
  
    
}
  
 
  
