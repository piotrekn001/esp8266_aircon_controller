#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);
const char* ssid = "espAP";
const char* password = "qwerty123";
void handleRoot() {
	server.send(200, "text/html", "<h1>You are connected</h1>");
}

  void setup()
  {
    Serial.begin(115200);
    Serial.println();
  
    Serial.print("Setting soft-AP ... ");
    
    IPAddress ip(192, 168, 43, 1);
    IPAddress gateway(192, 168, 43, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(ip, gateway, subnet);
    boolean result = WiFi.softAP(ssid, password);
    if(result == true)
    {
      Serial.println("Ready");
      IPAddress myIP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(myIP);
      server.on("/", handleRoot);
      server.begin();
      Serial.println("HTTP server started");
    }
    else
    {
      Serial.println("Failed!");
    }

  }
  
  void loop()
  {
    server.handleClient();
  }


