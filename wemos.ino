#include <ESP8266WiFi.h>          
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <SimpleTimer.h>
#include <ESP8266HTTPClient.h>
#include "ESP8266HTU21D.h"

/*
 *const int PIN_LED = 2; // D4 on NodeMCU and WeMos. Controls the onboard LED.
 *
 * Trigger for inititating config mode is Pin D3 and also flash button on NodeMCU
 * Flash button is convenient to use but if it is pressed it will stuff up the serial port device driver 
 * until the computer is rebooted on windows machines.
 */
String DEV_VERSION = "1.0";
const int REPOSITORY_INTERVAL = 12000; //Repository interval check, milisecond 5 per/min
const char* HOST = "http://iot.tpolis.com/sensor/set"; //Repository  full adress for remote job

const int TRIGGER_PIN = D3; // D3 on NodeMCU and WeMos.
const int LED_PIN = D7; //Diod pin
/*
 * Alternative trigger pin. Needs to be connected to a button to use this pin. It must be a momentary connection
 * not connected permanently to ground. Either trigger pin will work.
 * const int TRIGGER_PIN2 = 13; // D7 on NodeMCU and WeMos.
*/

bool initialConfig = false; // Indicates whether ESP has WiFi credentials saved from previous session
std::unique_ptr<ESP8266WebServer> server;
SimpleTimer timer_remote;
HTU21D HTU21D;

//-------------main functions----------------------------------------

String getTemp() {
      return String (HTU21D.readTemperature());
      
}
String getHum() {
      return String (HTU21D.readHumidity());
} 
void goblink(int i){ 
    if ((i%10) == 0){ 
  bool state = digitalRead(LED_PIN);
  digitalWrite(LED_PIN, !state);
   }
  }
void doblink(int pin, int count = 1, int timeout = 500){
    for(int i=0; i<count; i++){
         digitalWrite(pin, LOW);
         delay(timeout); 
         digitalWrite(pin, HIGH);
    }
  }
void sendState(){
          String temp = getTemp();
          delay(10);
          String hum = getHum();          
          HTTPClient http;
          http.begin(HOST);
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");
          http.POST("temp=" + temp + "&hum=" + hum);
          String response = http.getString();
          http.end();
          Serial.println(response);
          //CheckRemoteFeedback(response);  
 }
void CheckRemoteFeedback(String json){
     StaticJsonBuffer<200> jsonBuffer;
     JsonObject& jobj = jsonBuffer.parseObject(json);
     if (!jobj.success()){
         Serial.println("parseObject() failed");
         return;
         }
         //check server answer here. We are here only if json has been parsed successfuly
         String remote_job = jobj["remote_job"];
         String devise_id = jobj["devise_id"];
         if(remote_job[0] != '\0'){ 
         }       
         Serial.println(remote_job);
 }
 void sleepTime(){
      wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
      wifi_fpm_open();
      wifi_fpm_do_sleep(50*1000); //permanent sleep till triger gpio
 }
//------------- end main functions----------------------------------------  
void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, HIGH); //turn off led  
  
  Serial.begin(115200);
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  WiFi.printDiag(Serial); //Remove this line if you do not want to see WiFi password printed
  if (WiFi.SSID()==""){
    Serial.println("We haven't got any access point credentials, so get them now");   
    initialConfig = true;
  }
  else{
    WiFiManager wifiManager;
    wifiManager.getStatCred();
    WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
    unsigned long startedAt = millis();
    Serial.print("After waiting ");
    int connRes = WiFi.waitForConnectResult();
    float waited = (millis()- startedAt);
    Serial.print(waited/1000);
    Serial.print(" secs in setup() connection result is ");
    Serial.println(connRes);
  }

  if (WiFi.status()!=WL_CONNECTED){
    Serial.println("failed to connect, finishing setup anyway");
  } else{
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
    Serial.println("Starting http server...");
    server.reset(new ESP8266WebServer(WiFi.localIP(), 80));
        server->on("/stat", [](){
          String temp = getTemp();
          delay(10);
          String hum = getHum();
          server->send(200, "application/json", "{\"device_name\" : \"dht11\", \"version\" : " + DEV_VERSION + ", \"temp\" : " + temp + ", \"hum\" : " + hum + "}"); 
        });
        server->on("/temp", [](){ 
          sleepTime();
          server->send(200, "text/plain", getTemp()); 
        });
        server->on("/hum", [](){
          server->send(200, "text/plain", getHum());
        });
        server->begin();
        Serial.println("Custom HTTP server started");
        doblink(LED_PIN, 1, 1000);
        HTU21D.begin();
        timer_remote.setInterval(REPOSITORY_INTERVAL, sendState);

  }
}

void loop() { 
  // is configuration portal requested?
  if ((digitalRead(TRIGGER_PIN) == LOW) || (initialConfig)) {
    Serial.println("Configuration portal requested.");
     server.reset(new ESP8266WebServer(WiFi.localIP(), 80));
     
     digitalWrite(LED_BUILTIN, LOW); // turn the LED on by making the voltage LOW to tell us we are in configuration mode. Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //it starts an access point and goes into a blocking loop awaiting configuration
    if (!wifiManager.startConfigPortal("PetFeed")) {
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {
      //if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");
    }
    digitalWrite(LED_BUILTIN, HIGH); // Turn led off as we are not in configuration mode.
    ESP.reset(); // This is a bit crude. For some unknown reason webserver can only be started once per boot up so resetting the device allows to go back into config mode again when it reboots.
    delay(5000);
  }

  // put your main code here, to run repeatedly:
    if (WiFi.status()==WL_CONNECTED){
  server->handleClient();
  timer_remote.run();
    }
  // delay(1100);
}
