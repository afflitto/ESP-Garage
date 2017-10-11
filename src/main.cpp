#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <WebSocketsServer.h>
#include <ThingerWifi.h>
#include "pages.h"
#include "keys.h"

#define MDNS_NAME "garage"
#define BUTTON_DELAY 750 //Time in milliseconds that the relay is held HIGH

//const int relayPin = D1; //Pin used for the relay
const int relayPin = LED_BUILTIN; //Set to LED_BUILTIN for now for debugging

long relayStartTime = 0; //Used to keep track of how long the relay has been on

//Device Networking
MDNSResponder mdns;
ESP8266WebServer server(80);
WebSocketsServer ws = WebSocketsServer(81);
ThingerWifi thing(thingerUserName, thingerDeviceID, thingerDeviceCredential);

//Turns on the relay. Will turn off after BUTTON_DELAY ms
void activateRelay()
{
  relayStartTime = millis();

  if(relayStartTime + BUTTON_DELAY < BUTTON_DELAY)
  {
    Serial.println("overflow");//overflow
  }
}

//Turns off relay if needed
void updateRelay()
{
  if(millis() > relayStartTime + BUTTON_DELAY)
  {
    digitalWrite(relayPin, LOW);
  }
  else
  {
    digitalWrite(relayPin, HIGH);
  }
}

//URL Handlers
void handleRoot() {
  Serial.println("root");
  server.send(200, "text/html", rootHTML);
}

void handleManifest() {
  Serial.println("manifest");
  server.send(200, "text/json", manifest);
}

void handleNotFound() {
  Serial.println("not found");
  server.send(404, "text/plain", "404: not found");
}

//Websocket handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("ws disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("ws connected");
      break;
    case WStype_TEXT:
      Serial.printf("ws recieved: %s\n", payload);
      if(payload[0] == 't')
      {
        Serial.println("correct message");
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

  Serial.println("Connected");
  Serial.println("IP: " + WiFi.localIP().toString());

  //Add Thinger.io input resource
  //Will call activateRelay() when a web request is sent to Thinger.io
  thing["toggle"] << [](pson& in){
    activateRelay();
  };

  //Start MDNS
  if(mdns.begin(MDNS_NAME, WiFi.localIP()))
  {
    Serial.println("mdns started");
  }

  //Add URL handlers
  server.on("/", handleRoot);
  server.on("/manifest.json", handleManifest);
  server.onNotFound(handleNotFound);

  //Begin networking
  server.begin();
  Serial.println("web server started");
  ws.begin();
  ws.onEvent(webSocketEvent);
  Serial.println("websocket server started");
}

void loop() {
  //Update web server, web sockets, Thinger, and relay
  ws.loop();
  server.handleClient();
  thing.handle();
  updateRelay();
}
