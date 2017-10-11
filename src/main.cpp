#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <WebSocketsServer.h>
#include "pages.h"

#define BUTTON_DELAY 750

const int relayPin = LED_BUILTIN;

long relayStartTime = 0;

const char* ssid = "";
const char* password = "";
MDNSResponder mdns;

ESP8266WebServer server(80);
WebSocketsServer ws = WebSocketsServer(81);

void activateRelay()
{
  relayStartTime = millis();

  if(relayStartTime + BUTTON_DELAY < BUTTON_DELAY)
  {
    Serial.println("overflow");//overflow
  }
}

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
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected");
  Serial.println("IP: " + WiFi.localIP().toString());

  if(mdns.begin("esp8266", WiFi.localIP()))
  {
    Serial.println("mdns started");
  }

  server.on("/", handleRoot);
  server.on("/manifest.json", handleManifest);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("web server started");

  ws.begin();
  ws.onEvent(webSocketEvent);
  Serial.println("websocket server started");
}

void loop() {
  ws.loop();
  server.handleClient();

  updateRelay();
}
