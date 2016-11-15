#include <ESP8266WiFi.h>       // https://github.com/esp8266/Arduino

#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer


  // You need define ASYNC_WEB_SERVER before include WiFiManager
#define ASYNC_WEB_SERVER 1
#include <WiFiManager.h>

void setup() {
  
  Serial.begin(115200);
  Serial.println("\n Starting");
  
  Serial.println("Opening configuration portal");
  
  unsigned long startedAt = millis();
  
  
  
    // Setup AsyncWebServer at port 80 as internal server
  WiFiManager wifiManager(AsyncWebServer(80));
  
  
  

  //If no access point name has been previously entered disable timeout.
  if (WiFi.SSID()!="") wifiManager.setConfigPortalTimeout(60);
  
    //it starts an access point
    //and goes into a blocking loop awaiting configuration
  if (!wifiManager.startConfigPortal("ESP8266","password")) {//Delete these two parameters if you do not want a WiFi password on your configuration access point
    Serial.println("Not connected to WiFi but continuing anyway.");
  } else {
      //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
  }
  
  Serial.print("After waiting ");
  int connRes = WiFi.waitForConnectResult();
  float waited = (millis()- startedAt);
  Serial.print(waited/1000);
  Serial.print(" secs in setup() connection result is ");
  Serial.println(connRes);
  if (WiFi.status()!=WL_CONNECTED){
    Serial.println("failed to connect, finishing setup anyway");
  } else{
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  
}
