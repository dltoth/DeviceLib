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

#include <WiFiPortal.h>
#include <DeviceLib.h>

#define SOFT_AP_SSID "SleepingBear"
#define SOFT_AP_PSK  "BigLakeMI"
#define SERVER_PORT 80

#ifdef ESP8266
#define           BOARD "ESP8266"
#elif defined(ESP32)
#define          BOARD "ESP32"
#endif

WebContext       ctx;
WebContext*      svr = &ctx;
WiFiPortal       portal;
const char*      hostname = "hub";

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
 portal.logging(FINE);
 hub.logging(FINE);

/**
 *  Give Portal a hostname String to coordinate mDNS with the local router
 */
  portal.setHostname(hostname);

/**
 *  Initialize WiFiPortal with ssid and psk for the softAP, MUST be called prior to starting the connection sequence.
 */
  portal.setup(SOFT_AP_SSID,SOFT_AP_PSK);

/** Start WiFi with AP Portal, if successful the AP Portal will be discontinued and the application will be connected
 *  to an Access Point with SSID and PSK input from the portal. When completed, WiFi is in WIFI_STA mode and the softAP
 *  is disconnected. If a softAP is required for the application, set disconnectSoftAP(false) prior to starting the
 *  connection sequence. 
 *   
 *   Connection sequence is similar to ESP8266/ESP32 WiFi. This loop will return once valid SSID and PSK are provided by either:
 *   1. Successfully connecting with credentials cached by WiFi or
 *   2. Successfully connecting with credentials input from the portal interface
 *   Note the use of ConnectionState rather than WiFi status
 */
  while(portal.connectWiFi() != CNX_CONNECTED) {delay(500);}
  Serial.printf("\nWiFi Connected to %s with IP address: %s\n",WiFi.SSID().c_str(),WiFi.localIP().toString().c_str());

/** 
 *  The portal interface allows for hostname input, if provided start mDNS.
 */
  if(portal.hasHostName()) {
     Serial.printf("Starting mDNS on %s\n",portal.hostname());
     MDNS.begin(portal.hostname());    
  }
  
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
  hub.setup(svr);
  hub.addDevice(&c);

  Serial.printf("Starting main loop\n");
}

void loop() {
  ssdp.doSSDP();            // Handle SSDP queries
  ctx.handleClient();       // Handle HTTP requests
  hub.doDevice();           // Do a unit of work for the device
  updateMDNS();             // Update MDNS for ESP8266 if hostname is present on Connection String
}

void updateMDNS() {
#ifdef ESP8266
  if( portal.hasHostName() ) MDNS.update();
#endif
}
