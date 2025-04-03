/*
protože nemáme na wifi modul a esp by všechno neutáhlo tak nám esp slouží samo o sobě jen jako wifi
přijímá data která jsou tady v globálním scopu a mění se každých pár vteřin
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(3, 1); //3 = rx, 1 = tx


const char* ssid = "ESP8266_Access_Point"; 
const char* password = "12345678";        

ESP8266WebServer server(80); 

//měli jsme web pro debug
String deviceName = "ESP8266 Device";
int plyn =0;
int ohen=0;
float mq9;
float infra, mq2, mq4, teplota;
int co;

/*
mq9 - počet ppm
mq2 - % metanu
infra - cm vzdálenost ohně (pokud -1 oheň není)
teplota - teplota v °C
*/

void setup() {
  mySerial.begin(9600); 
  WiFi.softAP(ssid, password);  

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point IP Address: ");
  Serial.println(IP);

  //server pro deubg
  server.on("/", handleRoot); 
  server.begin();            
  Serial.println("Server started.");
}


//všechny informaci +-
void handleRoot() {
  String html = "<html><body>";
  html += "<h1>ESP8266 Web Server</h1>";
  html += "<p><strong>Device Name:</strong> " + deviceName + "</p>";
  html += "<p><strong>oxid monoxide:</strong> " + String(mq9) + " ppm</p>";
  html += "<p><strong>methane</strong> " + String(mq2) + " %</p>";
  html += "<p><strong>temperature</strong> " + String(teplota) + " °C</p>";
  html += "<p><strong>distance to fire</strong> " + String(infra) + " cm</p>";
  html += "</body></html>";

  server.send(200, "text/html", html); 
}

void loop() {
  if (mySerial.available()) {
    String data = mySerial.readStringUntil('\n');
 

    sscanf(data.c_str(), "%f,%f,%f,%f,%f,%d,%d,%d", &infra, &mq2, &mq4, &mq9, &teplota, &ohen, &plyn, &co);
    delay(500);

    } 
  server.handleClient();
}
