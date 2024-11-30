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

#include "ToggleDevice.h"

#define AP_SSID "Dumbledore 2.0"
#define AP_PSK  "2badboys"
#define SERVER_PORT 80

/**
 *   Conditional compilation for either ESP8266 or ESP32
 */
#ifdef ESP8266
#include <ESP8266WiFi.h>
#define           BOARD "ESP8266"
#elif defined(ESP32)
#include <WiFi.h>
#define          BOARD "ESP32"
#endif

/**
 *   Device hierarchy will consist of an ExtendedDevice (root) and a Custom Control (ToggleDevice)
 */
WebContext       ctx;
ExtendedDevice   root;
ToggleDevice     t;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println();
  Serial.printf("Starting ToggleDevice Test for Board %s\n",BOARD);

  WiFi.begin(AP_SSID,AP_PSK);
  Serial.printf("Connecting to Access Point %s\n",AP_SSID);
  while(WiFi.status() != WL_CONNECTED) {Serial.print(".");delay(500);}

  Serial.printf("WiFi Connected to %s with IP address: %s\n",WiFi.SSID().c_str(),WiFi.localIP().toString().c_str());

  ctx.begin(SERVER_PORT);
  Serial.printf("Web Server started on %s:%d/\n",WiFi.localIP().toString().c_str(),ctx.getLocalPort());

  root.setDisplayName("Root Device");
  root.setTarget("root");  
  root.addDevice(&t);

/**
 *  Set up the device hierarchy and register HTTP request handlers
 */
  root.setup(&ctx);

}

void loop() {
  ctx.handleClient();
  root.doDevice();
}
