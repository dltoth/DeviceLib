/**
 * 
 *  UPnPDevice Library
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

#include "ClockWithConfig.h"

#define AP_SSID "Dumbledore 2.0"
#define AP_PSK  "2badboys"
#define SERVER_PORT 80

#ifdef ESP8266
#include <ESP8266WiFi.h>
#define           BOARD "ESP8266"
#elif defined(ESP32)
#include <WiFi.h>
#define          BOARD "ESP32"
#endif

WebContext       ctx;
ExtendedDevice   root;
ClockDevice      c;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println();
  Serial.printf("Starting Sensor Test for Board %s\n",BOARD);

  WiFi.begin(AP_SSID,AP_PSK);
  Serial.printf("Connecting to Access Point %s\n",AP_SSID);
  while(WiFi.status() != WL_CONNECTED) {Serial.print(".");delay(500);}

  Serial.printf("WiFi Connected to %s with IP address: %s\n",WiFi.SSID().c_str(),WiFi.localIP().toString().c_str());

  ctx.begin(SERVER_PORT);
  Serial.printf("Web Server started on %s:%d/\n",WiFi.localIP().toString().c_str(),ctx.getLocalPort());

  c.setTimezone(-5.0);

  root.addDevice(&c);
  root.setDisplayName("Root Device");
  root.setTarget("root");  
  root.setup(&ctx);
  
  Instant start = c.now();
  char tBuff[64];
  char dBuff[64];
  start.printTime(tBuff,64);
  start.printDate(dBuff,64); 
  Serial.printf("CustomSensor started at %s on %s\n",tBuff,dBuff); 

/**
 *  
 * Print UPnP Info about RootDevice, Services, and embedded Devices
 * 
 */
  UPnPDevice::printInfo(&root);
  
}

void loop() {
  ctx.handleClient();
  root.doDevice();
}
