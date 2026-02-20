// שם מלא: מור גואטה ת.ז.: 314813379
#include <WiFiClient.h>
#include <WiFiUdp.h> 
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

extern int curCase;
extern bool manualRequestFromUser;
extern bool forceStart;

const char* ssid = "Shoshana1";
const char* pswd = "0535204659";

const char* mqtt_server = "10.0.0.5";
WiFiClient espClient;
PubSubClient mqttClient(espClient);
const int port = 3671;

void wifi_Setup() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, pswd);
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED && counter < 50) {
        delay(500);
        Serial.print(".");
        counter++;
    }
    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to network!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed!");
    }
    mqttClient.setServer(mqtt_server, 1883);
    mqttClient.setCallback(mqtt_callback);
}

int sendJson(String json) {
    if (WiFi.status() != WL_CONNECTED) return -1;
    HTTPClient http;
    String url = "http://" + String(mqtt_server) + ":" + String(port) + "/esp/create";
    http.begin(espClient, url); 
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(json);
    if(httpCode > 0) {
        Serial.printf("HTTP Response code: %d\n", httpCode);
    } else {
        Serial.printf("HTTP Post failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    return httpCode;
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
  if (String(topic) == "pot/1001/mode") {
    int newMode = message.toInt();
    if (newMode >= 1 && newMode <= 4) {
      curCase = newMode;
      Serial.print("Mode changed to: ");
      Serial.println(curCase);
    }
  }

  if (String(topic) == "pot/1001/command") {
    if (message == "START") manualRequestFromUser = true;
    if (message == "STOP") manualRequestFromUser = false;
    if (message == "FORCE_ON") forceStart = true;
  }
  Serial.print("DEBUG: Received something on topic: ");
  Serial.println(topic);
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqtt_server);
    if (mqttClient.connect("ESP32_Pot_1001")) { 
      Serial.println("...connected!");
      mqttClient.subscribe("pot/1001/mode");
      mqttClient.subscribe("pot/1001/command");
    } else {
      Serial.print("...failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 10 seconds");
      delay(10000);
    }
  }
}

int sendWaterLog(String json) {
    if (WiFi.status() != WL_CONNECTED) return -1;
    HTTPClient http;
    String url = "http://" + String(mqtt_server) + ":" + String(port) + "/esp/water-log";
    http.begin(espClient, url); 
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(json);
    if(httpCode > 0) {
        Serial.printf("Water Log Sent! Response: %d\n", httpCode);
    } else {
        Serial.printf("Water Log Failed: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    return httpCode;
}