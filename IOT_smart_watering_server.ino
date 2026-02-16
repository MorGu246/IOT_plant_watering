//עבודה עם שרת מרוחק
#include <WiFiClient.h>
#include <WiFiUdp.h> 
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

const char* ssid = "Shoshana1";//"Kinneret College";//Shoshana1
const char* pswd = "0535204659";//"55555333";//0535204659

const char* mqtt_server = "10.9.25.238";//"10.9.1.6"; //10.9.1.107//10.9.25.238//10.0.0.5
WiFiClient espClient;
PubSubClient mqttClient(espClient);
const int port = 3671;

// WiFiClient client;
// int server_port = 80;//http

void wifi_Setup() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, pswd); // הוספתי את הסיסמה כאן
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
}
// לכתוב פונקציה ב-IOT
// מתחת ל - wifi connected
// פונקציה לשליחת JSON ב-HTTP (לוגים של חיישנים)
int sendJson(String json) {
    if (WiFi.status() != WL_CONNECTED) return -1;
    HTTPClient http;
    // בניית ה-URL בצורה דינמית
    String url = "http://" + String(mqtt_server) + ":" + String(port) + "/esp/create/";
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
// פונקציה שתצטרך להוסיף כדי לקבל פקודות מה-MQTT
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    // כאן תכתוב את הלוגיקה שתשנה את curCase לפי מה שתשלח מה-VS Code
    // לדוגמה: אם הגיע "1" בטופיק "pot/case", אז curCase = 1
}