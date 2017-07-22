#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "./DNSServer.h"  

const byte        DNS_PORT = 53;          // Capture DNS requests on port 53
IPAddress         apIP(10, 10, 10, 1);    // Private network for server
DNSServer         dnsServer;              // Create the DNS object
ESP8266WebServer  webServer(80);          // HTTP server

String responseHTML = ""
                      "<html>"
                      "<style>"
    "body {"
        "background: black }"
    "section {"
        "background: white;"
        "color: black;"
        "border-radius: 1em;"
        "padding: 1em;"
        "position: absolute;"
        "top: 50%;"
        "left: 50%;"
        "margin-right: -50%;"
        "transform: translate(-50%, -50%) }"
  "</style>"
  "<section>"
    "<h1>Mess with the best, die like the rest.</h1>"
  "</section>";
 
extern "C" {
#include<user_interface.h>
}
 
int connectedClients = 0;
 
long nextCheck = 0;     //Keeps track of the value of millis to only check for new Clients every 5 seconds.
 
Adafruit_SSD1306 display = Adafruit_SSD1306();
 
/* configuration  wifi */
const char *ssid = "CableWifi";
const char *password = "";
 
#if defined(ESP8266)
  #define BUTTON_A 0
  #define BUTTON_B 16
  #define BUTTON_C 2
  #define LED      0
#endif
 
#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


void setup() {  
 
  delay(1000);
 
  //Initialize the display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.setRotation(2);
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  
  initializeAP();  //Setup the Huzzah as an access point
}
 
void initializeAP() {
 
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  // replay to all requests with same HTML
  webServer.onNotFound([]() {
    webServer.send(200, "text/html", responseHTML);
  });
  webServer.begin();
 
  //I've noticed that most times the huzzah doesn't let clients connect
  //as soon as it starts up. Here I give it 5 seconds to "warm up"
  //While it does this it prints 
  
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Initializing n00b");
  display.setCursor(0,10);
  display.print("hunting expedition");
 
  //The two bytes that define the bitmap for the loading dots
  static const unsigned char PROGMEM waiting_dot[] =
  { B00011000,
    B00011000 };
  
  for (int i = 0; i < 10; i++) {
 
    display.drawBitmap(i * 10, 20, waiting_dot, 8, 2, WHITE);
    display.display();
 
    delay(1000);
  }
  
  
  display.clearDisplay();
  display.display();
  display.setCursor(0,0);
  
  displaySSID();
  
}
 
void loop() {
 
  if (millis() >= nextCheck) {
    
    client_status();
 
    nextCheck = millis() + 5000;

    dnsServer.processNextRequest();
    webServer.handleClient();
    
  }
   
}
 
void displaySSID() {
 
  display.setTextSize(1);
  display.clearDisplay();
  
  display.setCursor(0,0);
  
  //display.print("AP SSID: ");
  
  display.setTextSize(2);
  //display.setCursor(0, 10);
  display.println("Searching");
  display.println("for n00bs");
  display.display();
      
}
 
void client_status() {
 
  unsigned char number_client;
  struct station_info *stat_info;
 
  struct ip_addr *IPaddress;
  IPAddress address;
  int i=1;
  
  number_client= wifi_softap_get_station_num(); // Count of stations which are connected to ESP8266 soft-AP
  stat_info = wifi_softap_get_station_info();
 
  if (number_client > connectedClients) {
     display.setTextSize(1);
     display.clearDisplay();
     display.setCursor(0, 0);
     display.setTextSize(2);
     display.println("n00b");
     display.println("Connected!");
     display.display();  
     delay(2000);
  } 
 
  if (number_client < connectedClients) {
     display.setTextSize(1);
     display.clearDisplay();
     display.setCursor(0, 0);
     display.setTextSize(2);
     display.println("n00b");
     display.println("Disconnected!");
     display.display();
     delay(2000); 
 
     if (number_client == 0) {
      displaySSID();
      
    }
    
  }
  
  connectedClients = number_client;
 
  while (stat_info != NULL) {
    
    display.clearDisplay();
    
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("n00b @ MAC:");
    //display.setTextSize(2);
    display.setCursor(0, 10);
    
    //Get the 6 octets of the client MAC address. Check to see if the value of each octet is less than
    //0xF. If it is we will print a leading 0 to avoid any confusion.
    if (stat_info->bssid[0] <= 0xF) {
      display.print("0");
    }
    
    display.print(stat_info->bssid[0],HEX);
    display.print(":");
    
    if (stat_info->bssid[1] <= 0xF) {
      display.print("0");
    }
    
    display.print(stat_info->bssid[1],HEX);
    display.print(":");
    
    if (stat_info->bssid[2] <= 0xF) {
      display.print("0");
    }
    
    display.print(stat_info->bssid[2],HEX);
    display.print(":");
    
    if (stat_info->bssid[3] <= 0xF) {
      display.print("0");
    
    }
    display.print(stat_info->bssid[3],HEX);
    display.print(":");
    
    if (stat_info->bssid[4] <= 0xF) {
      display.print("0");
    }
    
    display.print(stat_info->bssid[4],HEX);
    display.print(":");
    
    if (stat_info->bssid[5] <= 0xF) {
      display.print("0");
    }
    
    display.print(stat_info->bssid[5],HEX);
 
    display.display();
    
    stat_info = STAILQ_NEXT(stat_info, next);
    i++;
  
  }
}
