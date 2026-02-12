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
unsigned long lastSensorSend = 0;
bool motorOn = false;

bool my_manual = false;
// int ShabatOn;
// int ShabatOff;
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

// הצהרה על פונקציות (כדי שה-Compiler יכיר אותן)
void startMotor();
void stopMotor();
void manageWeatherMode(float temp, int light);
void manageSoilMode(int humSoil, int light);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  wifi_Setup();
  dht.begin();
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
float humSoil = analogRead(Humidity);
float light = map(analogRead(LDRPin), 0, 4095, 0, 1024);
// sendJson(buildJson("hum",hum));
// delay(21600000);
if (millis() - lastSensorSend > 3600000) { 
      sendJson(buildJson("temp", temp));
      lastSensorSend = millis();
  }
curCase=1;
  switch(curCase) {
      case 1: // Weather Mode
        manageWeatherMode(temp, (int)light);
        break;
      case 2: // Soil Moisture Mode
        manageSoilMode(humSoil, (int)light);
        break;
      case 3: // Manual Mode
        // // כאן הלוגיקה תלויה בהודעות MQTT שמשנות את my_manual
        // if(my_manual && (light < 700)) {
        //     digitalWrite(motor_B1A, HIGH);
        // } else {
        //     digitalWrite(motor_B1A, LOW);
        // }
        if (manualRequestFromUser) { 
            if (light > 700 && !forceStart) { 
                // sendMqttStatus("WARNING_HIGH_LIGHT"); 
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
        // שימוש ב-NTPClient לבדיקת שעה מדויקת
        break;
    }
  
  // case 1:
  // //timeOfSend=millis();
  //   if(temp<26){ // טמפרטורה לא גבוהה
  //     if(!motorOn && millis() - timeOfSendOff >= 7200000){ //2 שעות עובד
  //       if(light<700){
  //         digitalWrite(motor_B1A, HIGH);
  //         digitalWrite(motor_B1B, LOW);
  //         motorOn = true;
  //         timeOfSendOn = millis();
  //         sendJson(buildJson("temp",temp));//<== לשלוח ערך חיישן טמפרטורה
  //       }
  //     }
  //     if(motorOn && millis() - timeOfSendOn >= 36000000){ //10 שעות לא עובד
  //         digitalWrite(motor_B1A, LOW);
  //         digitalWrite(motor_B1B, LOW);
  //         motorOn = false;
  //         timeOfSendOff = millis();
  //         sendJson(buildJson("temp",temp));//<== לשלוח ערך חיישן טמפרטורה
  //       }
  //     }
  //     if(temp>=26){ //טמפרטורה גבוהה
  //     if(!motorOn && millis() - timeOfSendOff >= 10800000){ //3 שעות עובד
  //       if(light<700){
  //         digitalWrite(motor_B1A, HIGH);
  //         digitalWrite(motor_B1B, LOW);
  //         motorOn = true;
  //         timeOfSendOn = millis();
  //         sendJson(buildJson("temp",temp));//<== לשלוח ערך חיישן טמפרטורה
  //       }
  //     }
  //     if(motorOn && millis() - timeOfSendOn >= 10800000){ //3 שעות לא עובד
  //         digitalWrite(motor_B1A, LOW);
  //         digitalWrite(motor_B1B, LOW);
  //         motorOn = false;
  //         timeOfSendOff = millis();
  //         sendJson(buildJson("temp",temp));//<== לשלוח ערך חיישן טמפרטורה
  //       }
  //     }
  //   break;
  // case 2:
  // timeOfSend=millis();
  //   if(hum>60){ // לחות גבוהה
  //         if(millis()-timeOfSend >= 7200000){ //2 שעות עובד
  //           if(light<700){
  //             digitalWrite(motor_B1A, HIGH);
  //             digitalWrite(motor_B1B, LOW);
  //             timeOfSend=millis();
  //           }
  //         }
  //         if(millis()-timeOfSend >= 36000000){ //10 שעות לא עובד
  //             digitalWrite(motor_B1A, LOW);
  //             digitalWrite(motor_B1B, LOW);
  //             timeOfSend=millis();
  //           }
  //         }
  //   if(hum<60){ // לחות לא גבוהה
  //         if(millis()-timeOfSend >= 10800000){ //3 שעות עובד
  //           if(light<700){
  //             digitalWrite(motor_B1A, HIGH);
  //             digitalWrite(motor_B1B, LOW);
  //             timeOfSend=millis();
  //           }
  //         }
  //         if(millis()-timeOfSend >= 32400000){ //9 שעות לא עובד
  //             digitalWrite(motor_B1A, LOW);
  //             digitalWrite(motor_B1B, LOW);
  //             timeOfSend=millis();
  //           }
  //         }
  //   break;
  // case 3:
  //   if(light>700){
  //     Serial.print(" Not a good idea, the water will evaporize ");
  //     if(my_manual){
  //       digitalWrite(motor_B1A, HIGH);
  //       digitalWrite(motor_B1B, LOW);
  //     }
  //     else{
  //       digitalWrite(motor_B1A, LOW);
  //       digitalWrite(motor_B1B, LOW);
  //     }
  //   }
  //   break;
  // case 4:
  //   // digitalWrite(motor_B1A, HIGH);
  //   // digitalWrite(motor_B1B, LOW);
  //   // delay(1000);
  //   // digitalWrite(motor_B1A, LOW);
  //   // digitalWrite(motor_B1B, LOW);
  //   // delay(2000);
  //   timeOfSend=millis();
  //   if(temp<26){ // טמפרטורה לא גבוהה
  //     if(millis()-timeOfSend >= ShabatOn){ //2 שעות עובד
  //       if(light<700){
  //         digitalWrite(motor_B1A, HIGH);
  //         digitalWrite(motor_B1B, LOW);
  //         timeOfSend=millis();
  //       }
  //     }
  //     if(millis()-timeOfSend >= ShabatOff){ //10 שעות לא עובד
  //         digitalWrite(motor_B1A, LOW);
  //         digitalWrite(motor_B1B, LOW);
  //         timeOfSend=millis();
  //       }
  //     }
  // }
  //long waterAmount=0;
}
void manageWeatherMode(float temp, int light) {
    // הגדרת זמנים לפי טמפרטורה (מעל 26 = קיץ, מתחת = חורף)
    unsigned long intervalOn = (temp >= 26) ? 10800000 : 7200000; // 3 שעות בקיץ, שעתיים בחורף
    // חישוב זמן מנוחה כדי להגיע בדיוק למספר הפעמים ביום:
    // קיץ: 3 שעות עבודה + 5 שעות מנוחה = מחזור של 8 שעות -> 3 פעמים ביממה.
    // חורף: שעתיים עבודה + 10 שעות מנוחה = מחזור של 12 שעות -> פעמיים ביממה.
    unsigned long intervalOff = (temp >= 26) ? 18000000 : 36000000; 
    // אם המנוע כבוי ועבר זמן המנוחה הדרוש
    if (!motorOn && (millis() - timeOfSendOff >= intervalOff)) {
        if (light < 700) { // הגנת אור חזק
            startMotor();
        }
    } 
    // אם המנוע פועל ועבר זמן ההשקיה הנדרש
    else if (motorOn && (millis() - timeOfSendOn >= intervalOn)) {
        stopMotor();
    }
}
void startMotor() {
  if(!motorOn) {
    digitalWrite(motor_B1A, HIGH);
    digitalWrite(motor_B1B, LOW);
    motorOn = true;
    timeOfSendOn = millis(); // שומרים מתי התחלנו
    Serial.println("Motor STARTED");
  }
}
void stopMotor() {
  if(motorOn) {
    digitalWrite(motor_B1A, LOW);
    digitalWrite(motor_B1B, LOW);
    motorOn = false;
    timeOfSendOff = millis(); // שומרים מתי סיימנו
    // כאן אנחנו מחשבים את משך ההשקיה בפועל עבור הדרישה של "לוג השקיות"
    unsigned long durationMs = timeOfSendOff - timeOfSendOn;
    float durationMinutes = durationMs / 60000.0;
    Serial.print("Motor STOPPED. Irrigation lasted: ");
    Serial.print(durationMinutes);
    Serial.println(" minutes.");
    // כאן תוסיף קריאה לפונקציה ששולחת את durationMinutes לשרת ה-REST/MQTT שלך!
    // sendIrrigationLogToSever(durationMinutes);
  }
}
void manageSoilMode(int humSoil, int light) {
    // הגדרת ספים (כויל לפי הבדיקה שעשית עם האצבעות)
    const int DRY_THRESHOLD = 800;   // מתחת לזה האדמה יבשה - צריך להשקות
    const int WET_THRESHOLD = 2500;  // מעל זה האדמה רטובה - אפשר להפסיק
    const int LIGHT_LIMIT = 700;     // סף האור להגנה
    // 1. תנאי הפעלה: אדמה יבשה וגם אין שמש חזקה
    if (!motorOn && (humSoil < DRY_THRESHOLD)) {
        if (light < LIGHT_LIMIT) {
            startMotor();
        }
    }
    // 2. תנאי כיבוי: האדמה נהייתה רטובה מספיק
    // הערה: במטלה לא ביקשו לכבות בגלל אור באמצע השקיית לחות, 
    // אבל זה מומלץ כדי לחסוך במים (הגנה על המערכת).
    else if (motorOn && (humSoil > WET_THRESHOLD || light > LIGHT_LIMIT)) {
        stopMotor();
    }
}
  case 3: // Manual Mode
    if (manualRequestFromUser) { // משתנה שמתעדכן מה-MQTT
        if (light > 700 && !forceStart) { 
            // כאן אתה שולח הודעה חזרה ל-MQTT/VS Code: "Warning: High light!"
            // ורק אם המשתמש לוחץ "אישור" (שמעדכן את forceStart ל-true) המנוע יפעל
            sendMqttStatus("WARNING_HIGH_LIGHT");
            stopMotor(); 
        } else {
            startMotor();
        }
    } else {
        stopMotor();
        forceStart = false; // איפוס האישור המיוחד
    }
    break;