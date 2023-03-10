/**
 * 
 *  DeviceLib Library
 *  Copyright (C) 2023  Daniel L Toth
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
 
#include <ssdp.h>
#include <WiFiPortal.h>
#include "Thermometer.h"
#include "Hydrometer.h"
#include "HumidityFan.h"
#include "OutletTimer.h"
#include "SoftwareClock.h"
#include "ExtendedDevice.h"

using namespace lsc;

#define SOFT_AP_SSID "SleepingBear"
#define SOFT_AP_PSK  "BigLakeMI"
#define SERVER_PORT 80

#ifdef ESP8266
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
ESP8266WebServer  server(SERVER_PORT);
ESP8266WebServer* svr = &server;
#define           BOARD "ESP8266"
#elif defined(ESP32)
#include <ESPmDNS.h>
#include <WebServer.h>
WebServer        server(SERVER_PORT);
WebServer*       svr = &server;
#define          BOARD "ESP32"
#endif

WebContext       context;
WebContext*      ctx = &context;
WiFiPortal       portal;

SSDP             ssdp;
ExtendedDevice   root;
SoftwareClock    sw_clock;
Thermometer      t;
Hydrometer       h;
HumidityFan      f;
OutletTimer      o;
const char*      hostname = "BigBang";


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println();
  Serial.printf("Starting DeviceLib Test for Board %s\n",BOARD);

/**
 *  Send progress to Serial. LoggingLevel can be NONE, INFO, FINE, or FINEST
 */
  portal.logging(FINE);

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

/** 
 *  The portal interface allows for hostname input, if provided start mDNS.
 */
  if(portal.hasHostName()) {
     Serial.printf("Starting mDNS on %s\n",portal.hostname());
     MDNS.begin(portal.hostname());    
  }

  Serial.printf("\nWiFi Connected to %s with IP address: %s\n",WiFi.SSID().c_str(),WiFi.localIP().toString().c_str());
/**
 *  Set timezone to Eastern Standard time
 */
  sw_clock.setTimezone(-5);
  sw_clock.setDisplayName("Clock");

/**
 *  Set display name to something recognizable. 
 */
  root.setDisplayName("Extended Device");

/**
 *  Initialize SSDP services
 *  ssdp.logging(FINEST);
 */
  ssdp.begin(&root);

/**
 * Setup Web Context for serving the HTML UI. 
 */
  server.begin();
  ctx->setup(svr,WiFi.localIP(),SERVER_PORT);
  Serial.printf("Web Server started on %s:%d/\n",ctx->getLocalIPAddress().toString().c_str(),ctx->getLocalPort());
  
  root.setTarget("testDevice");
  root.setup(ctx);
  
/**
 *  Late binding for embedded devices; setup will be called for each device as  
 *  they are added. Alternavively, embedded devices can be added prior to RootDevice 
 *  setup. In either case, Sensors are displayed in the order that they are added.
 *  Note that device target must be set prior to calling setup or adding to the 
 *  RootDevice.
 */
  root.addDevices(&f,&o,&t,&h,&sw_clock);

/**
 *  
 * Print UPnP Info about RootDevice, Services, and embedded Devices
 * 
 */
  RootDevice::printInfo(&root);

}

void loop() {
  ssdp.doSSDP();            // Handle SSDP queries
  server.handleClient();    // Handle HTTP requests
  root.doDevice();          // Do a unit of work for the device
  updateMDNS();             // Update MDNS for ESP8266 if hostname is present on Connection String
}

void updateMDNS() {
#ifdef ESP8266
  if( portal.hasHostName() ) MDNS.update();
#endif
}

