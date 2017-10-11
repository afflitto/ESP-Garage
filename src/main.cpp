#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <WebSocketsServer.h>
#include "pages.h"

const char* ssid = "";
const char* password = "";
MDNSResponder mdns;

ESP8266WebServer server(80);
WebSocketsServer ws = WebSocketsServer(81);

void handleRoot() {
  Serial.println("root");
  server.send(200, "text/html", rootHTML);
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
      break;
  }
}

void setup() {
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
}
