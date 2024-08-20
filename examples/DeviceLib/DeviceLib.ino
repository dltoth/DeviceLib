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
 
#include <WiFiPortal.h>
#include "DeviceLib.h"

#define SOFT_AP_SSID "SleepingBear"
#define SOFT_AP_PSK  "BigLakeMI"
#define SERVER_PORT   80

#ifdef ESP8266
#define           BOARD "ESP8266"
#elif defined(ESP32)
#define          BOARD "ESP32"
#endif

WebContext       context;
WiFiPortal       portal;

SSDP             ssdp;
ExtendedDevice   root;


SoftwareClock    c;
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
  c.setTimezone(-5.0);
  c.setDisplayName("Clock");

/**
 *  Set display name to something recognizable. 
 */
  root.setDisplayName("NTP Clock Test");

/**
 *  Initialize SSDP services
 *  ssdp.logging(FINEST);
 */
  ssdp.begin(&root);

/**
 *  Setup Web Context for serving the HTML UI. 
 */
  context.begin();
  Serial.printf("Web Server started on %s:%d/\n",WiFi.localIP().toString().c_str(),context.getLocalPort());

  c.setTimezone(-5);
  c.setDisplayName("System Clock");
  c.setNTPSync(60);                  // Sync with the NTP server evern hour

/**
 *  Both HumidityFan and OutletControl derive from RelayControl which sets a default Pin to D5 (GPIO pin 14), 
 *  so one of the devices has to redefine the Pin number:
 *      HumidityFan - WEMOS_D7 (GPIO 13)
 *      OutletTimer - WEMOS_D5 (GPIO pin 14)
 *  and this must be done prior to setup()
 */
  f.pin(WEMOS_D7);
  
  root.setTarget("root");
  root.setDisplayName("Extended Device");
  root.setup(&context);
   
/**
 *  Late binding for embedded devices; setup will be called for each device as  
 *  they are added. Alternavively, embedded devices can be added prior to RootDevice 
 *  setup. In either case, Devices are displayed in the order that they are added.
 *  Note that since device target is used in setting HandlerFunctions on the WebServer in setup(), target 
 *  must be set prior to calling setup or adding to the RootDevice.
 */
  root.addDevices(&f,&o,&t,&h,&c);


  char dateBuff[64];
  c.initializationDate().printDateTime(dateBuff,64);
  Serial.printf("Software Clock initialized to %s\n",dateBuff);
  c.now().printDateTime(dateBuff,64);
  Serial.printf("Software Clock Test started at %s\n",dateBuff);

/**
 *  
 * Print UPnP Info about RootDevice, Services, and embedded Devices
 * 
 */
//  RootDevice::printInfo(&root);

}

void loop() {
  ssdp.doSSDP();            // Handle SSDP queries
  context.handleClient();   // Handle HTTP requests
  root.doDevice();          // Do a unit of work for the device
  updateMDNS();             // Update MDNS for ESP8266 if hostname is present on Connection String
}

void updateMDNS() {
#ifdef ESP8266
  if( portal.hasHostName() ) MDNS.update();
#endif
}


