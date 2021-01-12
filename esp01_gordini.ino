#include <ESP8266WiFi.h>
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <FS.h>

#define DEBUG false

//GPIO output
#define POSITION_LIGHTS 0
#define MAIN_LIGHTS 1
#define FOG_LIGHTS 2
#define BLINKER_LIGHTS 3

IPAddress ip(192,168,0,1);
IPAddress gw(192,168,0,1);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);

unsigned long latestTime = 0;
boolean blinkerActive = false;
int blinkerState = LOW;

void log(String log) {
  if (DEBUG) {
    Serial.println(log);
  }
}

void initSpiffs() {
  log("Init SPIFFS");
  if(!SPIFFS.begin()){
     log("Error occurred while mounting SPIFFS");
     return;
  }
}

void initGpio() {
  log("Init GPIO");
  
  pinMode(POSITION_LIGHTS, OUTPUT);
  pinMode(MAIN_LIGHTS, OUTPUT);
  pinMode(FOG_LIGHTS, OUTPUT);
  pinMode(BLINKER_LIGHTS, OUTPUT);
  
  digitalWrite(POSITION_LIGHTS, LOW);
  digitalWrite(MAIN_LIGHTS, LOW);
  digitalWrite(FOG_LIGHTS, LOW);
  digitalWrite(BLINKER_LIGHTS, LOW);
}

void initWifi() {
  log("Init Wifi");
  log("Setting soft-AP ... ");

  if(WiFi.softAP("esp-gordini")) {
    log("AP Wifi ready!");
  } else {
    log("AP Wifi failed!");
  }
  WiFi.softAPConfig(ip, gw, subnet);
  delay(100);
}

void initWebServer() {
  log("Init Web server");
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    handleOn(request);
  });
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    handleOff(request);
  });
  server.onNotFound([](AsyncWebServerRequest *request){
    loadFromSpiffs(request->url(), request);
  });  
  
  server.begin();
  log("HTTP server started");
}

void setup() {
  if (DEBUG) {
    Serial.begin(115200);
  }
  log("");

  initSpiffs();
  initWifi();
  initWebServer(); 
  if (!DEBUG) {
     initGpio();
  }

  latestTime = millis();
}

void loop() {
  unsigned long currentTime = millis();

  log("blink active: " + String(blinkerActive));
  //delay(1000);
  
  if (blinkerActive == true) {
    log("diff: " + String(currentTime-latestTime));
    if (currentTime-latestTime >= 750) {
      latestTime = currentTime;
      blinkerState = !blinkerState;
      log("blink state: " + String(blinkerState));
      if (!DEBUG) {
        digitalWrite(BLINKER_LIGHTS, blinkerState);
      }
    }
  } else {
    if (!blinkerState) {
      digitalWrite(BLINKER_LIGHTS, LOW);
    }    
  }
}

void loadFromSpiffs(String path, AsyncWebServerRequest *request) {
  String dataType = "text/plain";
  if (path.endsWith("/")) {
    path += "index.html";
  }

  if (path.endsWith(".html") || path.endsWith(".htm")){
    dataType = "text/html";
  }
  else if (path.endsWith(".css")) {
    dataType = "text/css";
  }
  else if (path.endsWith(".js")) {
    dataType = "application/javascript";
  }
  else if (path.endsWith(".png")) {
    dataType = "image/png";
  }
  else if (path.endsWith(".jpg")) {
    dataType = "image/jpeg";
  }
  /*
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";
  */  
  
  request->send(SPIFFS, path, dataType);
}

void handleOn(AsyncWebServerRequest *request) {
  String id = request->getParam("id")->value();  
  log("ON:" + id);
  led(id.toInt(), HIGH);
}

void handleOff(AsyncWebServerRequest *request) {
  String id = request->getParam("id")->value();  
  log("OFF:" + id);
  led(id.toInt(), LOW);
} 

void led(int gpio, int state) {
  if (gpio<0 || gpio>3 ) {
    log("Invalid gpio number:" + gpio);
  } else {
    if (gpio == BLINKER_LIGHTS) {
      blinkerState = state;
      if (state == HIGH) {
        blinkerActive = true;
      } else if (state == LOW) {
        blinkerActive = false;  
      }
    }
    
    if (!DEBUG && gpio!=BLINKER_LIGHTS) {
      digitalWrite(gpio, state);
    }
  }
}
