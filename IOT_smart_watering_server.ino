//עבודה עם שרת מרוחק
#include <WiFiClient.h>
#include <WiFiUdp.h> 
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

const char* ssid = "Kinneret College";//"Kinneret College";//Shoshana1
const char* pswd = "0535204659";//"55555333";//0535204659

const char* mqtt_server = "10.9.25.238";//"10.9.1.6"; //10.9.1.107//10.9.25.238//10.0.0.5
WiFiClient espClient;
PubSubClient client(espClient);

// WiFiClient client;
// int server_port = 80;//http

void wifi_Setup() {
            Serial.println("wifiSetup");
            WiFi.begin(ssid);
        //    WiFi.begin(ssid,pswd);
        
            while (WiFi.status() != WL_CONNECTED) {
                Serial.println("trying ...");
                delay(100);
            }
            Serial.println("Connected to network");
}
////////////////////////////////////////////////////////////
// const char* ssid = "Shoshana1";
// const char* password = "0535204659";
// const char* ipAddress = "10.0.0.30";
// using namespace websockets;

// void wifi_Setup() {
//   Serial.println("wifiSetup");
//   WiFi.begin(ssid,password);

//   while (WiFi.status() != WL_CONNECTED) {
//     Serial.println("trying ...");
//     delay(100);
//   }
//   Serial.println("Connected to network");
// }
////////////////////////////////////////////////////////////
// לכתוב פונקציה ב-IOT
// מתחת ל - wifi connected
int sendJson(String json){
  HTTPClient http;
  http.begin(espClient,"http://10.9.25.238:3671/esp/create/"); //10.9.25.238//10.0.0.5
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(json);
  if(httpCode == HTTP_CODE_OK){
    Serial.println("HTTP response code ");
    Serial.println(httpCode);
  }
  http.end();
  return httpCode;
}