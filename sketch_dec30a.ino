// שם מלא: מור גואטה ת.ז.: 314813379
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <dummy.h>
#include <DHT.h>

extern PubSubClient mqttClient;
void reconnectMQTT();

#define motor_B1A 22
#define motor_B1B 23
#define DHTTYPE DHT22
#define DHT_PIN 16
DHT dht(DHT_PIN, DHTTYPE);
#define LDRPin 36
#define Humidity 39

bool manualRequestFromUser = false;
bool forceStart = false;

unsigned long timeOfSendOn = 0;
unsigned long timeOfSendOff = 0;
unsigned long lastSensorSend = 0;
bool motorOn = false;

bool my_manual = false;
int curCase = 1;
const int id_pot = 1001;

String buildJson(String sensor, float val){
  JsonDocument doc;
  String json;
  doc["name"] = sensor;
  doc["avg"] = val;
  doc["potId"] = id_pot;
  serializeJson(doc, json);
  return json;
}

void startMotor();
void stopMotor();
void manageWeatherMode(float temp, int light);
void manageSoilMode(int humSoil, int light);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  wifi_Setup();
  delay(2000);
  reconnectMQTT();
  dht.begin();
  pinMode(motor_B1A, OUTPUT);
  pinMode(motor_B1B, OUTPUT);
  sendJson(buildJson("temp",23.23));
  digitalWrite(motor_B1A, LOW);
  digitalWrite(motor_B1B, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
float temp = dht.readTemperature();
float humidity = dht.readHumidity();
float humSoil = analogRead(Humidity);
float light = map(analogRead(LDRPin), 0, 4095, 0, 1024);
if (millis() - lastSensorSend > 21600000L) { 
      sendJson(buildJson("temp", temp));
      delay(100);
      sendJson(buildJson("soil_hum", humSoil));
      delay(100);
      sendJson(buildJson("light", light));
      lastSensorSend = millis();
  }
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
  switch(curCase) {
      case 1: // Weather Mode
        manageWeatherMode(temp, (int)light);
        break;
      case 2: // Soil Moisture Mode
        manageSoilMode(humSoil, (int)light);
        break;
      case 3: // Manual Mode
        if (manualRequestFromUser) {
          if (light > 700 && !forceStart) { 
            Serial.println("Warning: High light intensity!");
            stopMotor(); 
          } else {
            startMotor();
            }
        } else {
            stopMotor();
            forceStart = false;
        }
      break;
      case 4: // Scheduled Mode
        manageScheduledMode();
        break;
    }
}
void manageWeatherMode(float temp, int light) {
    unsigned long intervalOn = (temp >= 26) ? 10800000 : 7200000;
    unsigned long intervalOff = (temp >= 26) ? 18000000 : 36000000; 
    if (!motorOn && (millis() - timeOfSendOff >= intervalOff)) {
        if (light < 700) {
            startMotor();
        }
    } 
    else if (motorOn && (millis() - timeOfSendOn >= intervalOn)) {
        stopMotor();
    }
}
void startMotor() {
  if(!motorOn) {
    digitalWrite(motor_B1A, HIGH);
    digitalWrite(motor_B1B, LOW);
    motorOn = true;
    timeOfSendOn = millis();
    Serial.println("Motor STARTED");
  }
}
void stopMotor() {
  if(motorOn) {
    digitalWrite(motor_B1A, LOW);
    digitalWrite(motor_B1B, LOW);
    motorOn = false;
    timeOfSendOff = millis();
    unsigned long durationMs = timeOfSendOff - timeOfSendOn;
    float durationMinutes = durationMs / 60000.0;
    Serial.print("Motor STOPPED. Irrigation lasted: ");
    Serial.print(durationMinutes);
    Serial.println(" minutes.");
    String logJson = "{\"potId\":" + String(id_pot) + ",\"duration\":" + String(durationMinutes) + "}";
    sendWaterLog(logJson);
  }
}
void manageSoilMode(int humSoil, int light) {
    const int DRY_THRESHOLD = 800;
    const int WET_THRESHOLD = 2500;
    const int LIGHT_LIMIT = 700;
    // 1. תנאי הפעלה: אדמה יבשה וגם אין שמש חזקה
    if (!motorOn && (humSoil < DRY_THRESHOLD)) {
        if (light < LIGHT_LIMIT) {
            startMotor();
        }
    }
    // 2. תנאי כיבוי: האדמה נהייתה רטובה מספיק
    else if (motorOn && (humSoil > WET_THRESHOLD || light > LIGHT_LIMIT)) {
        stopMotor();
    }
}
void manageScheduledMode() {
    const unsigned long intervalOn = 300000L;
    const unsigned long intervalOff = 43200000L;
    if (!motorOn && (millis() - timeOfSendOff >= intervalOff)) {
        Serial.println("Scheduled Mode: Starting periodic watering...");
        startMotor();
    }
    else if (motorOn && (millis() - timeOfSendOn >= intervalOn)) {
        Serial.println("Scheduled Mode: Finishing periodic watering...");
        stopMotor();
    }
}