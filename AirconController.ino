#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DNSServer.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>

IRsend irsend(4);  // An IR LED is controlled by GPIO pin 4 (D2)

typedef struct
{
        unsigned char mode : 3;
        unsigned char onOff : 1;
        unsigned char fan : 2;
        unsigned char pos : 1;
        unsigned char sleep : 1; // 8
        unsigned char temp : 4; // 12
        unsigned char unknown1 : 8; // 20
        unsigned char turbo : 1;
        unsigned char light : 1;
        unsigned char pine : 1;
        unsigned char xfan : 1; // 24
        unsigned char home : 1; // 25
        unsigned char unknown2 : 8;
        unsigned char unknown3 : 2; // 35
        unsigned char posSide : 4; // 39
        unsigned char posBottom : 4; // 43
        unsigned char tempout : 1; // 44
        unsigned char tempin : 1; // 45
        unsigned char ifeel : 1; // 46
        unsigned char unknown4 : 8; // 52
        unsigned char unknown5 : 8; // 60
        unsigned char unknown6 : 2; // 62
} __attribute__((packed)) packet;

uint8_t calcCrc(packet *p) {
  unsigned long long crcMask;
  packet pMask;
  memset(&pMask, 0, 8);
  pMask.mode = 0x7;
  pMask.onOff = 0x1;
  pMask.temp = 0xf;
  pMask.posBottom = 0xf;
  pMask.home = 0x1;
  memcpy(&crcMask, &pMask, 8);

  unsigned long long ppp;
  memcpy(&ppp, p, 8);
  ppp = ppp & crcMask;

  unsigned long long calcCrc = (((ppp >> 0)  & 0x0f) +
                                ((ppp >> 8)  & 0x0f) +
                                ((ppp >> 24) & 0x0f) +
                                ((ppp >> 39) & 0x0f) +
                                       0x0a) & 0x0f;

  return (uint8_t) calcCrc;
}

const char* ssid = "esp8266";
const char* password = "qwerty123";
const byte        DNS_PORT = 53;          // Capture DNS requests on port 53
IPAddress         apIP(10, 10, 10, 10);    // Private network for server
DNSServer         dnsServer;              // Create the DNS object 
// TCP server at port 80 will respond to HTTP requests
ESP8266WebServer server(80);

void sendUpdate(uint8_t mode, uint8_t temp) {
  packet p;
  memset(&p, 0, 8);
  p.mode = mode;
  p.onOff = 1;
  p.temp = temp - 16;

  p.light = 1;
  p.pine = 1;
  p.unknown2 = 40;
  p.unknown3 = 1;
  uint8_t crc = calcCrc(&p);
  unsigned long long pp;
  
  memcpy(&pp, &p, 8);
  
  uint16_t rawData[139];
  int i = 0;
  rawData[i++] = 9040;
  rawData[i++] = 4440;

  for (int j = 0; j <= 34; j++) {
    rawData[i++] = 696;
    if ((pp >> j) & 0x01) {
      rawData[i++] = 1600;
    }
    else {
      rawData[i++] = 510;
    }
  }

  rawData[i++] = 696;
  rawData[i++] = 20000;

  for (int j = 35; j <= 62; j++) {
    rawData[i++] = 696;
    if ((pp >> j) & 0x01) {
      rawData[i++] = 1600;
    }
    else {
      rawData[i++] = 510;
    }
  }

  for (int j = 0; j <= 3; j++) {
    rawData[i++] = 696;
    if ((crc >> j) & 0x01) {
      rawData[i++] = 1600;
    }
    else {
      rawData[i++] = 510;
    }
  }

  rawData[i++] = 696;

  irsend.sendRaw(rawData, 139, 38);  // Send a raw data capture at 38kHz.
}

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

  Serial.println("TCP server started"); // 
}

  void loop() {
    dnsServer.processNextRequest(); // procesowanie requesta przez serwer dns
    server.handleClient(); // procesowanie requesta przez serwer http
  }

void notFound() {
  server.send(200, "text/html", "<h1>Wrong request</h1>");
}
void handleRoot() { // przykładowy query string http://www.aircon.com/?mode=1&temp=1
  if(server.args() == 2) // walidacja ilości argumentów
  {
    if(server.argName(0) == "mode" && server.argName(1) == "temp") //walidacja nazw i kolejności argumentów
    {
      String modeParamString = server.arg(0);
      int mode = atoi(modeParamString.c_str());
      String tempParamString = server.arg(1);
      int temp = atoi(tempParamString.c_str());
      if(modeParamString != "0" && mode == 0 or tempParamString != "0" && temp == 0 ) // walidacja typów argumentów 
      {
        server.send(200, "text/html", "Invalid query parameters"); // odpowiedź z informacją o błedzie.
        Serial.println("Invalid query parameters"); // log do serial monitora o błędzie
      }
      else
      {

          String validationErrors = "";
          if(IsValid(mode, temp, validationErrors))
          {          
            if (SendData(mode, temp))  // wysłanie danych 
            {
              server.send(200, "text/html", "OK"); // informacja o poprawnie wykonanej operacji
              Serial.print("Temp set to: "); // log do serial monitora o ustawionych parametrach.
              Serial.print(temp);
              Serial.print(", Mode set to: ");
              Serial.println(mode);

            }
            else
            {
              server.send(200, "text/html", "Something went wrong"); // odpowiedź z informacją o błedzie.
              Serial.println("Something went wrong"); // log do serial monitora o błędzie
            }
          }
          else
          {
            server.send(200, "text/html", validationErrors); // odpowiedź z informacją o błedzie.
            Serial.println("Validation errors"); // log do serial monitora o błędach walidacji
          }
      }
    }
    else
    {
      server.send(200, "text/html","Invalid query parameters" ); // odpowiedź z informacją o błedzie.
      Serial.println("Invalid query parameters"); // log do serial monitora o błędzie
    }
  }
  else 
  {
    server.send(200, "text/html", "Invalid number of query parameters");
    Serial.println("Invalid number query parameters"); // log do serial monitora o błędzie
  }
}

bool SendData(int mode, int temp)
{
  //TODO: Send data to ir diode
  return true;
}




bool IsValid(int mode, int temp, String& validationErrors)
{
  bool isValid = true;
  //TODO Ustawić poprawne warunki
  if(mode < 0 || mode > 3 )
  {
    isValid = false;
    validationErrors = validationErrors + "<h1>Mode should have value between 0 and 3</h1> \r\n";
  }
  if(temp< 16 || temp > 31)
  {
    isValid = false;
    validationErrors = validationErrors + "<h1>Temp should have value between 16 and 31</h1> \r\n";
  }
  return isValid;
}




