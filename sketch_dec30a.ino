#include <ArduinoJson.h>
//#include <ArduinoJson.hpp>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

#include <dummy.h>
#include <DHT.h>
#define motor_B1A 22
#define motor_B1B 23
#define DHTTYPE DHT22       //עבור חיישן טמפרטורה
#define DHT_PIN 16          //עבור חיישן טמפרטורה
DHT dht(DHT_PIN, DHTTYPE);  //עבור חיישן טמפרטורה
#define LDRPin 36
#define Humidity 39
unsigned long timeOfSend=millis();

unsigned long timeOfSendOn = 0;
unsigned long timeOfSendOff = 0;
bool motorOn = false;

bool my_manual;
int ShabatOn;
int ShabatOff;
int curCase;
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  wifi_Setup();
  pinMode(motor_B1A, OUTPUT);
  pinMode(motor_B1B, OUTPUT);
  sendJson(buildJson("temp",23.23));
  //sendJson(buildJson("hum",hum));
}

void loop() {
  // put your main code here, to run repeatedly:
digitalWrite(motor_B1A, LOW);
digitalWrite(motor_B1B, LOW);
float temp = dht.readTemperature();
float humidity = dht.readHumidity();
float hum = analogRead(Humidity);
float light = map(analogRead(LDRPin), 0, 1023, 0, 255);
sendJson(buildJson("hum",hum));
delay(1000000);
curCase=1;
  switch(curCase){
  /*default:
    timeOfSend=millis();
    if(temp<26){ // טמפרטורה לא גבוהה
      if(millis()-timeOfSend > 7200000){ //2 שעות עובד
        if(light<700){
          digitalWrite(motor_B1A, HIGH);
          digitalWrite(motor_B1B, LOW);
          timeOfSend=millis();
        }
      }
      if(millis()-timeOfSend > 36000000){ //10 שעות לא עובד
          digitalWrite(motor_B1A, LOW);
          digitalWrite(motor_B1B, LOW);
          timeOfSend=millis();
        }
      }
      if(temp>=31){ //טמפרטורה גבוהה
      if(millis()-timeOfSend > 10800000){ //3 שעות עובד
        if(light<700){
          digitalWrite(motor_B1A, HIGH);
          digitalWrite(motor_B1B, LOW);
          timeOfSend=millis();
        }
      }
      if(millis()-timeOfSend > 10800000){ //3 שעות לא עובד
          digitalWrite(motor_B1A, LOW);
          digitalWrite(motor_B1B, LOW);
          timeOfSend=millis();
        }
      }
    break;*/
  case 1:
  //timeOfSend=millis();
    if(temp<26){ // טמפרטורה לא גבוהה
      if(!motorOn && millis() - timeOfSendOff >= 7200000){ //2 שעות עובד
        if(light<700){
          digitalWrite(motor_B1A, HIGH);
          digitalWrite(motor_B1B, LOW);
          motorOn = true;
          timeOfSendOn = millis();
          sendJson(buildJson("temp",temp));//<== לשלוח ערך חיישן טמפרטורה
        }
      }
      if(motorOn && millis() - timeOfSendOn >= 36000000){ //10 שעות לא עובד
          digitalWrite(motor_B1A, LOW);
          digitalWrite(motor_B1B, LOW);
          motorOn = false;
          timeOfSendOff = millis();
          sendJson(buildJson("temp",temp));//<== לשלוח ערך חיישן טמפרטורה
        }
      }
      if(temp>=26){ //טמפרטורה גבוהה
      if(!motorOn && millis() - timeOfSendOff >= 10800000){ //3 שעות עובד
        if(light<700){
          digitalWrite(motor_B1A, HIGH);
          digitalWrite(motor_B1B, LOW);
          motorOn = true;
          timeOfSendOn = millis();
          sendJson(buildJson("temp",temp));//<== לשלוח ערך חיישן טמפרטורה
        }
      }
      if(motorOn && millis() - timeOfSendOn >= 10800000){ //3 שעות לא עובד
          digitalWrite(motor_B1A, LOW);
          digitalWrite(motor_B1B, LOW);
          motorOn = false;
          timeOfSendOff = millis();
          sendJson(buildJson("temp",temp));//<== לשלוח ערך חיישן טמפרטורה
        }
      }
    break;
  case 2:
  timeOfSend=millis();
    if(hum>60){ // לחות גבוהה
          if(millis()-timeOfSend >= 7200000){ //2 שעות עובד
            if(light<700){
              digitalWrite(motor_B1A, HIGH);
              digitalWrite(motor_B1B, LOW);
              timeOfSend=millis();
            }
          }
          if(millis()-timeOfSend >= 36000000){ //10 שעות לא עובד
              digitalWrite(motor_B1A, LOW);
              digitalWrite(motor_B1B, LOW);
              timeOfSend=millis();
            }
          }
    if(hum<60){ // לחות לא גבוהה
          if(millis()-timeOfSend >= 10800000){ //3 שעות עובד
            if(light<700){
              digitalWrite(motor_B1A, HIGH);
              digitalWrite(motor_B1B, LOW);
              timeOfSend=millis();
            }
          }
          if(millis()-timeOfSend >= 32400000){ //9 שעות לא עובד
              digitalWrite(motor_B1A, LOW);
              digitalWrite(motor_B1B, LOW);
              timeOfSend=millis();
            }
          }
    break;
  case 3:
    if(light>700){
      Serial.print(" Not a good idea, the water will evaporize ");
      if(my_manual){
        digitalWrite(motor_B1A, HIGH);
        digitalWrite(motor_B1B, LOW);
      }
      else{
        digitalWrite(motor_B1A, LOW);
        digitalWrite(motor_B1B, LOW);
      }
    }
    break;
  case 4:
    // digitalWrite(motor_B1A, HIGH);
    // digitalWrite(motor_B1B, LOW);
    // delay(1000);
    // digitalWrite(motor_B1A, LOW);
    // digitalWrite(motor_B1B, LOW);
    // delay(2000);
    timeOfSend=millis();
    if(temp<26){ // טמפרטורה לא גבוהה
      if(millis()-timeOfSend >= ShabatOn){ //2 שעות עובד
        if(light<700){
          digitalWrite(motor_B1A, HIGH);
          digitalWrite(motor_B1B, LOW);
          timeOfSend=millis();
        }
      }
      if(millis()-timeOfSend >= ShabatOff){ //10 שעות לא עובד
          digitalWrite(motor_B1A, LOW);
          digitalWrite(motor_B1B, LOW);
          timeOfSend=millis();
        }
      }
  }
  //long waterAmount=0;
}