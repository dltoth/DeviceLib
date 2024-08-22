/**
 *  From Boilerplate Test Harness
 *  Conditional Compilation for both ESP8266 and ESP32.
 */

#include <ExtendedDevice.h>
#include <RelayControl.h>
#include <ssdp.h>
#include <SoftwareClock.h>
#include <WiFiPortal.h>

using namespace lsc;

#define SOFT_AP_SSID "SleepingBear"
#define SOFT_AP_PSK  "BigLakeMI"
#define SERVER_PORT 80

#ifdef ESP8266
#include <ESP8266mDNS.h>
#define           BOARD "ESP8266"
#elif defined(ESP32)
#include <ESPmDNS.h>
#define          BOARD "ESP32"
#endif

WebContext       ctx;
WebContext*      svr = &ctx;
WiFiPortal       portal;

SSDP             ssdp;
ExtendedDevice   root;
SoftwareClock    c;
RelayControl     relay;
const char*      deviceName = "Outlet";

/**
 *   Test for Relay  
 */

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println();
  Serial.printf("\nStarting %s for Board %s\n",deviceName,BOARD);

/**
 *   Set logging level to FINE
 */
  relay.logging(WARNING);
  portal.logging(WARNING);

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
     Serial.printf("\nStarting mDNS on %s\n",portal.hostname());
     MDNS.begin(portal.hostname());    
  }

  Serial.printf("\nWiFi Connected to %s with IP address: %s\n",WiFi.SSID().c_str(),WiFi.localIP().toString().c_str());

/**
 *  Set timezone to Eastern Daylight Time
 */
  c.setTimezone(-5);

/**
 *  Set client display name to something recognizable from "Nearby Devices".
 */
  root.setDisplayName(deviceName);
  relay.setDisplayName("Smart Outlet");

/**
 *  Initialize SSDP services
 */
  ssdp.begin(&root);
  ssdp.logging(WARNING);

/**
 * Setup Web Context for serving the HTML UI. 
 */
  ctx.begin(SERVER_PORT);
  Serial.printf("Web Server started on %s:%d/\n",WiFi.localIP().toString().c_str(),ctx.getLocalPort());

/**
 * setup() will register HTTP handlers based on target, so setTarget() must always be called prior to setup(), otherwise default target will be used.
 */
  root.setTarget("device");
  root.setup(svr);

/**
 *  Late binding for embedded devices; setup will be called for each device as  
 *  they are added. Alternavively, embedded devices can be added prior to RootDevice 
 *  setup. In either case, Sensors are displayed in the order that they are added.
 *  Note that device target must be set prior to calling setup or adding to the 
 *  RootDevice.
 *  Device pin definitions must be set prior to the call to setup.
 */
  root.addDevices(&relay,&c);

/**
 *  WEMOS D1 by default turns the LED ON, set pin 4 to HIGH to turn it OFF.
 */
  pinMode(WEMOS_D4,OUTPUT);
  digitalWrite(WEMOS_D4,HIGH);
  Serial.printf("LED turned OFF... heading into loop()\n");

}

void loop() {

/**
 *  Do a unit of work for the device
 */
  root.doDevice();

/**
 *  Handle SSDP queries
 */
  ssdp.doSSDP();

/**
 *  Handle HTTP requests
 */
  ctx.handleClient();

/**
 *  Update MDNS for ESP8266 if hostname is present on Connection String
 */
  updateMDNS(); 

}

void updateMDNS() {
#ifdef ESP8266
  if( portal.hasHostName() ) MDNS.update();
#endif
}
