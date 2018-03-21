#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DNSServer.h"   

const char* ssid = "esp8266";
const char* password = "qwerty123";
const byte        DNS_PORT = 53;          // Capture DNS requests on port 53
IPAddress         apIP(10, 10, 10, 10);    // Private network for server
DNSServer         dnsServer;              // Create the DNS object 
// TCP server at port 80 will respond to HTTP requests
ESP8266WebServer server(80);

void setup(void)
{  
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  // configure access point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid,password); // WiFi name
  dnsServer.start(DNS_PORT, "www.AIRCON.com", apIP);
  // Wait for connection
  server.onNotFound(notFound);
  server.on("/", handleRoot);
  server.begin();

  Serial.println("TCP server started");
}

  void loop() {
    dnsServer.processNextRequest();
    server.handleClient();
  }

void notFound() {
  server.send(200, "text/html", "<h1>Wrong request</h1>");
}
void handleRoot() {
	server.send(200, "text/html", "<h1>You are connected</h1>");
}



