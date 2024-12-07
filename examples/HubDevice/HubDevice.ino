/**
 * 
 *  DeviceLib Library
 *  Copyright (C) 2024  Daniel L Toth
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published 
 *  by the Free Software Foundation, either version 3 of the License, or any 
 *  later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  
 *  The author can be contacted at dan@leelanausoftware.com  
 *
 */

#include <DeviceLib.h>

#define AP_SSID "My_SSID"
#define AP_PSK  "My_PSK"
#define SERVER_PORT 80

#ifdef ESP8266
#define           BOARD "ESP8266"
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#define          BOARD "ESP32"
#include <ESPmDNS.h>
#endif

MDNSResponder    mDNS;
WebContext       ctx;
const char*      hostname = "hub-test";

SSDP             ssdp;
HubDevice        hub;
SoftwareClock    c;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.printf("\n\n");
  Serial.printf("Starting HubDevice for Board %s\n",BOARD);
  Serial.printf("\n\n");

/**
 *  Send progress to Serial. LoggingLevel can be NONE, INFO, FINE, or FINEST
 */
  hub.logging(FINE);

  Serial.printf("Connecting to Access Point %s\n",AP_SSID);
  WiFi.begin(AP_SSID,AP_PSK);
  while(WiFi.status() != WL_CONNECTED) {Serial.print(".");delay(500);}

  Serial.printf("\nWiFi Connected to %s with IP address: %s\n",WiFi.SSID().c_str(),WiFi.localIP().toString().c_str());

/** 
 *  Start mDNS.
 */
  Serial.printf("Starting mDNS on %s\n",hostname);
  MDNS.begin(hostname);    
  
/**
 *  Set timezone to Eastern Standard time
 */
  c.setTimezone(-5);

/**
 *  Set client display name to something recognizable from HubDevice. 
 */
  hub.setDisplayName("Device Hub");

/**
 *  Initialize SSDP services
 */
  Serial.printf("Starting SSDP\n");
  ssdp.begin(&hub);

/**
 * Setup Web Context for serving the HTML UI. 
 */
  ctx.begin(SERVER_PORT);
  Serial.printf("Web Server started on %s:%d/\n",WiFi.localIP().toString().c_str(),ctx.getLocalPort());

  hub.setTarget("hub");
  hub.setup(&ctx);
  hub.addDevice(&c);
}

void loop() {
  ssdp.doSSDP();            // Handle SSDP queries
  ctx.handleClient();       // Handle HTTP requests
  hub.doDevice();           // Do a unit of work for the device
  updateMDNS();             // Update MDNS for ESP8266 if hostname is present on Connection String
}

void updateMDNS() {
#ifdef ESP8266
  MDNS.update();
#endif  
}
