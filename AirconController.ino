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




