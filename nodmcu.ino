#include <ESP8266WiFi.h>          
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <Servo.h>

extern "C" {
  #include "user_interface.h"
}



Servo servo;
       
/*
 *const int PIN_LED = 2; // D4 on NodeMCU and WeMos. Controls the onboard LED.
 *
 * Trigger for inititating config mode is Pin D3 and also flash button on NodeMCU
 * Flash button is convenient to use but if it is pressed it will stuff up the serial port device driver 
 * until the computer is rebooted on windows machines.
 */
const int TRIGGER_PIN = D3; // D3 on NodeMCU and WeMos.
const int LED_PIN = D7; //Diod pin
const int MOTOR_PIN = D5; //Motor pin
/*
 * Alternative trigger pin. Needs to be connected to a button to use this pin. It must be a momentary connection
 * not connected permanently to ground. Either trigger pin will work.
 *
 * const int TRIGGER_PIN2 = 13; // D7 on NodeMCU and WeMos.
*/

// Indicates whether ESP has WiFi credentials saved from previous session
bool initialConfig = false;

std::unique_ptr<ESP8266WebServer> server;
//-------------main functions----------------------------------------
  void goFeed(String sval) {
        int ival = sval.toInt();
        servo.write(ival);
          char msg[100];
          strcpy(msg, "Servo has been moved to : ");
          strcat(msg, sval.c_str());
          server->send(200, "text/plain", msg);
          Serial.println(sval);
  }
  void goFeedSlow(String sval) {
        int ival = sval.toInt();
        for(int a=0; a<ival; a++){
              for(int i=0; i<180; i++){
                 goblink(i);
                 servo.write(i);
                 delay(10);
                 }
                 for(int i=180; i>0; i--){
                 goblink(i);
                 servo.write(i);
                 delay(5);
                 }
                 delay(200); // for fill the bank))
                 //servo.write(0);   // replaced by for for noise reduce
                 //delay(500);
        }
          char msg[100];
          strcpy(msg, "Performed cycles : ");
          strcat(msg, sval.c_str());
          server->send(200, "text/plain", msg);
          Serial.println(sval);
  }
  void goblink(int i){ 
    if ((i%10) == 0){ 
  bool state = digitalRead(LED_PIN);
  digitalWrite(LED_PIN, !state);
   }
  }
  void checkRepo(int interval){
    
  }
//------------- end main functions----------------------------------------  
void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, HIGH); //turn off led  
  int start_time = millis(); // remember starttime
  servo.attach(MOTOR_PIN);
  servo.write(0); //ставим вал под 0
  // put your setup code here, to run once:
  

  Serial.begin(115200);
  Serial.print("\n Starting at :");
  Serial.println(start_time);
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
        server->on("/whoami", [](){ 
          server->send(200, "text/plain", "{\"device_name\" : \"PetFeeder\", \"version\" : \"1.0\"}"); 
        });
        server->on("/servo", [](){ 
          goFeed(server->arg("val")); 
        });
        server->on("/servoslow", [](){
          goFeedSlow(server->arg("val")); 
        });
        server->begin();
    Serial.println("Custom HTTP server started");
  
  }
}


void loop() {
  // is configuration portal requested?
  if ((digitalRead(TRIGGER_PIN) == LOW) || (initialConfig)) {
    Serial.println("Configuration portal requested.");
     server.reset(new ESP8266WebServer(WiFi.localIP(), 80));
     
     digitalWrite(LED_BUILTIN, LOW); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //it starts an access point 
    //and goes into a blocking loop awaiting configuration
    if (!wifiManager.startConfigPortal("GOFeeder")) {
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {
      //if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");
    }
    digitalWrite(LED_BUILTIN, HIGH); // Turn led off as we are not in configuration mode.
    ESP.reset(); // This is a bit crude. For some unknown reason webserver can only be started once per boot up 
    // so resetting the device allows to go back into config mode again when it reboots.
    delay(5000);
  }


  // put your main code here, to run repeatedly:
    if (WiFi.status()==WL_CONNECTED){
  server->handleClient();
    }
}
