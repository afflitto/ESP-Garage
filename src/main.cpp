//Comment out the following line to disable Thinger.io integration
//You can use Thinger to connect to IFTTT for Google Home/Alexa integration
#define USE_THINGER
//Uncomment the following line to enable Thinger debugging
//#define _DEBUG_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <WebSocketsServer.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#ifdef USE_THINGER
  #include <ThingerWifi.h>
#endif

#include "pages.h"  //Strings containing the HTML pages
#include "keys.h"   //WiFi and Thinger credentials

//Config:
#define MDNS_NAME "garage"          //MDNS name to connect to garage.local
#define BUTTON_DELAY 750            //Time in milliseconds that the relay is held HIGH
//const int relayPin = D1;          //Pin used for the relay
const int relayPin = LED_BUILTIN;   //Set to LED_BUILTIN for now for debugging


long relayStartTime = 0; //Used to keep track of how long the relay has been on
bool enableRelay = false; //disable relay during OTA

//Device Networking
MDNSResponder mdns;
ESP8266WebServer server(80);
WebSocketsServer ws = WebSocketsServer(81);
#ifdef USE_THINGER
  ThingerWifi thing(thingerUserName, thingerDeviceID, thingerDeviceCredential);
#endif

//Turns on the relay. Will turn off after BUTTON_DELAY ms
void activateRelay()
{
  enableRelay = true;

  relayStartTime = millis();

  if(relayStartTime + BUTTON_DELAY < BUTTON_DELAY)
  {
    Serial.println("overflow");//overflow, might need to fix later
  }
}

//Turns off relay if needed
void updateRelay()
{
  if(millis() > relayStartTime + BUTTON_DELAY)
  {
    digitalWrite(relayPin, LOW);
    enableRelay = false;
  }
  else if(enableRelay)
  {
    digitalWrite(relayPin, HIGH);
  }
}

//URL Handlers
void handleRoot() {
  Serial.println("Serving root webpage");
  server.send(200, "text/html", rootHTML);
}

void handleManifest() {
  Serial.println("Serving manifest.json");
  server.send(200, "text/json", manifest);
}

void handleNotFound() {
  Serial.println("404: not found: " + server.uri());
  server.send(404, "text/plain", "404: not found");
}

//Websocket handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("Websocket disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("Websocket connected");
      break;
    case WStype_TEXT:
      Serial.printf("Websocket recieved: %s\n", payload);
      if(payload[0] == 't')
      {
        Serial.println("Activating relay");
        activateRelay();
      }
      break;
  }
}


void setup() {
  //Set up relay pin
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  Serial.begin(115200);

  //Connect to WiFi
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi");
  Serial.println("IP: " + WiFi.localIP().toString());

  //Add Thinger.io input resource
  //Will call activateRelay() when a web request is sent to Thinger.io
  #ifdef USE_THINGER
    thing["toggle"] << [](pson& in){
      activateRelay();
    };
  #endif

  //Start MDNS
  if(mdns.begin(MDNS_NAME, WiFi.localIP()))
  {
    mdns.addService("http", "tcp", 80);
    String mdnsString = MDNS_NAME;
    Serial.println("MDNS started on " + mdnsString + ".local");
  }

  //Add URL handlers
  server.on("/", handleRoot);
  server.on("/manifest.json", handleManifest);
  server.onNotFound(handleNotFound);

  //ArduinoOTA Handlers
  ArduinoOTA.onStart([]() {
    digitalWrite(relayPin, LOW);
    Serial.println("Starting OTA...");
  });
  ArduinoOTA.onEnd([]() {
      Serial.println("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress * 100) / total);
  });
  ArduinoOTA.onError([](ota_error_t error){
    Serial.println(error);
  });

  //Begin networking
  server.begin();
  Serial.println("Web server started");
  ws.begin();
  ws.onEvent(webSocketEvent);
  Serial.println("Websocket server started");
  ArduinoOTA.begin();
  Serial.println("ArduinoOTA started");
}

void loop() {
  //Update web server, web sockets, Thinger, ArduinoOTA, and relay
  ws.loop();
  server.handleClient();
  ArduinoOTA.handle();
  updateRelay();
  #ifdef USE_THINGER
    thing.handle();
  #endif
}
