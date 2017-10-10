#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "";
const char* password = "";
MDNSResponder mdns;

ESP8266WebServer server(80);

void handleRoot() {
  Serial.println("root");
  server.send(200, "text/plain", "hello world");
}

void handleNotFound() {
  Serial.println("not found");
  server.send(404, "text/plain", "404: not found");
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
  Serial.println("server started");
}

void loop() {
  server.handleClient();
}
