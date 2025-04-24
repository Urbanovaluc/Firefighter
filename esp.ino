/*
protože nemáme na wifi modul a esp by všechno neutáhlo tak nám esp slouží samo o sobě jen jako wifi
přijímá data která jsou tady v globálním scopu a mění se každých pár vteřin
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

SoftwareSerial mySerial(3, 1); //3 = rx, 1 = tx

const char* serverUrl = "https://firefighterspot.vercel.app/api/data";
const char* ssid = "SEM JMÉNO WIFI"; 
const char* password = "SEM HESLO OD WIFI";        



//měli jsme web pro debug
String deviceName = "ESP8266 Device";
int plyn =0;
int ohen=0;
float mq9;
float infra, mq2, mq4, teplota;
int co;
bool isFire = false;
/*
mq9 - počet ppm
mq2 - % metanu
infra - cm vzdálenost ohně (pokud -1 oheň není)
teplota - teplota v °C
*/

void setup() {
  mySerial.begin(9600); 
  //Serial.begin(9600);
 // WiFi.softAP(ssid, password);  
//  IPAddress IP = WiFi.softAPIP();

WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());      


 // Serial.print("Access Point IP Address: ");
 // Serial.println(IP);

  //server pro deubg
 // server.on("/", handleRoot); 
 // server.begin();            
 // Serial.println("Server started.");
}


//všechny informaci +-
/*void handleRoot() {
  String html = "<html><body>";
  html += "<h1>ESP8266 Web Server</h1>";
  html += "<p><strong>Device Name:</strong> " + deviceName + "</p>";
  html += "<p><strong>oxid monoxide:</strong> " + String(mq9) + " ppm</p>";
  html += "<p><strong>methane</strong> " + String(mq2) + " %</p>";
  html += "<p><strong>temperature</strong> " + String(teplota) + " °C</p>";
  html += "<p><strong>distance to fire</strong> " + String(infra) + " cm</p>";
  html += "</body></html>";

  server.send(200, "text/html", html); 
}*/

void loop() {
  delay(5000);
  if (mySerial.available()) {
    String data = mySerial.readStringUntil('\n');
 

    sscanf(data.c_str(), "%f,%f,%f,%f,%f,%d,%d,%d", &infra, &mq2, &mq4, &mq9, &teplota, &ohen, &plyn, &co);
    if(infra > 70 || infra == -1){
      infra = -1;
      isFire = false;
    }else{
      isFire = true;
    }
    
     sendJsonData(mq2, infra, isFire, teplota, mq9);
    
    } 
  //server.handleClient();
 //sendJsonData(4, 7, true, 2, 5);
}

void sendJsonData(float methane, int fireDistance, bool isFire, float tempC, int coPpm) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();  // Skip certificate validation (ok for testing)
    HTTPClient http;

    http.begin(client,serverUrl);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> jsonDoc;
    jsonDoc["methanePercentage"] = methane;
    jsonDoc["fireDistanceCm"] = fireDistance;
    jsonDoc["isFirePresent"] = isFire;
    jsonDoc["temperatureCelsius"] = tempC;
    jsonDoc["coPpmCount"] = coPpm;

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    int httpResponseCode = http.POST(jsonString);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response:");
      Serial.println(response);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }}