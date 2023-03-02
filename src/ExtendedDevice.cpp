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
#include <UPnPBuffer.h>
#include "ExtendedDevice.h"

namespace lsc {

const char SSDP_Search[]                  PROGMEM = "M-SEARCH * HTTP/1.1\r\n"
                                                    "HOST: 239.255.255.250:1900\r\n"
                                                    "MAN: ssdp:discover\r\n"
                                                    "ST: upnp:rootdevice\r\n"
                                                    "ST.LEELANAUSOFTWARE.COM: \r\n"
                                                    "USER-AGENT: ESP8266 UPnP/1.1 LSC-SSDP/1.0\r\n\r\n";
const char LocationHeader[]               PROGMEM = "LOCATION";
const char nearby_title[]                 PROGMEM = "<H1 align=\"center\"> Devices Near %s </H1><br><br>";
const char ExtendedDevice_config_form[]      PROGMEM = "<form action=\"%s\"><div align=\"center\">"                                                      // Form Path
         "<label for=\"displayName\">Device Name &nbsp &nbsp</label>"
         "<input type=\"text\" placeholder=\"%s\" name=\"displayName\"><br><br><br>"                                                                     // DisplayName
         "<button class=\"fmButton\" type=\"button\" style=\"width:12em\" onclick=\"window.location.href=\'%s\';\">Reset Access Point</button><br><br>"  // Reset AP path
         "<button class=\"fmButton\" type=\"submit\">Submit</button>&nbsp&nbsp"
         "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button>"                                           // Cancel path
      "</div></form><br><br>";
const char ExtendedDevice_resetAP[]          PROGMEM = "<h3 align=\"center\"> Select OK to clear network credentials for %s </h3>"                          // ssid
                                                    "<h4 align=\"center\">Access Point can be reset on next device power cycle </h4><br><br>"
                                          "<div align=\"center\">"
                                             "<a href=\"%s\" class=\"small apButton\">OK</a>&nbsp&nbsp"                                                  // Clear credentials path
                                             "<a href=\"%s\" class=\"small apButton\">Cancel</a>"                                                        // Cancel path
                                          "</div></body></html>";
const char brk_html[]                   PROGMEM = "<br>";   
#define SSDP_BUFFER_SIZE 1000
#define DESC_HEADER_SIZE 150

/**
 *  Static RTT initialization
 */
 
INITIALIZE_STATIC_TYPE(ExtendedDevice);
INITIALIZE_UPnP_TYPE(ExtendedDevice,urn:LeelanauSoftware-com:device:ExtendedDevice:1);

ExtendedDevice::ExtendedDevice() : RootDevice("root") {
  addServices(getConfiguration(),setConfiguration());   // Add services for configuration
  setDisplayName("Extended Device");
  setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
}

ExtendedDevice::ExtendedDevice(const char* target) : RootDevice(target) {
  addServices(getConfiguration(),setConfiguration());   // Add services for configuration
  setDisplayName("Extended Device");
  setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
}

void ExtendedDevice::display(WebContext* svr) {

  char buffer[DISPLAY_SIZE];
  int size = sizeof(buffer);

/** Add HTML Header and Title with Display Name
 */
  int pos = formatHeader(buffer,size,getDisplayName());

/** Add a button for each embedded device
 *  
 */
  char pathBuff[100];
  for( int i=0; (i<_numDevices) && (size>0); i++ ) {
    UPnPDevice* d = device(i);
    if( d != NULL ) {
       d->getPath(pathBuff,100);
       pos = formatBuffer_P(buffer,size,pos,app_button,pathBuff,d->getDisplayName());
    }
  }

/** Add a Config button 
 */
  setConfiguration()->formPath(pathBuff,100);
  pos = formatBuffer_P(buffer,size,pos,brk_html);
  pos = formatBuffer_P(buffer,size,pos,config_button,pathBuff,"Configure");

/** Add the HTML tail
 */ 
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer);
}

/**
 *   *** Needs to incorporate an iframe for display of controls.
 */
void ExtendedDevice::displayRoot(WebContext* svr) {
  char buffer[DISPLAY_SIZE];
  int size = sizeof(buffer);

/** Add HTML Header and Title with Display Name
 */
  int pos = formatHeader(buffer,size,getDisplayName());

/** Add Content 
 */
  formatContent(buffer+pos,size-pos);
  pos = strlen(buffer);

/** Add a Nearby Devices button that will display all nearby RootDevices as buttons
 */
  char pathBuff[100];
  handlerPath(pathBuff,100,"nearbyDevices");
  pos = formatBuffer_P(buffer,size,pos,app_button,pathBuff,"Nearby Devices");
  
/** Add the HTML tail
 */ 
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer);
}

void ExtendedDevice::nearbyDevices(WebContext* svr) {
  
  char buffer[DISPLAY_SIZE];
  int size = sizeof(buffer);

  IPAddress remote = svr->getRemoteIPAddress();
  String ssidStr;
  IPAddress ifc;
  if(SSDP::isSoftAPIP(remote)) {
    ssidStr = WiFi.softAPSSID();
    ifc = WiFi.softAPIP();
  }
  else {
    ssidStr = WiFi.SSID();
    ifc = WiFi.localIP();
  }

/** Add HTML Title with Display Name
 */
  char nearbyTitle[50];
  snprintf(nearbyTitle,50,"Devices on %s",ssidStr.c_str());
  int pos = formatHeader(buffer,size,nearbyTitle);

/** Search Subnet for nearby RootDevices
 *  
 */
  char* buff = buffer;
  int timeout = 3000;
  SSDP::searchRequest("upnp:rootdevice",([buff,size,&pos](UPnPBuffer* b){
       char name[32];
       if( b->displayName(name,32) ) {
           char loc[64];
           if( b->headerValue_P(LocationHeader,loc,64) ) pos = formatBuffer_P(buff,size,pos,app_button,loc,name);                 
       }  
    }),ifc,timeout);
   
/** Add the HTML tail
 */ 
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer);
}

void ExtendedDevice::configForm(WebContext* svr) {
  char buffer[1500];
  int size = sizeof(buffer);
  
/**
 *    Config Form HTML Start with Title
 */
  char title[50];
  snprintf(title,50,"Set %s Configuration",getDisplayName());
  int pos = formatHeader(buffer,size,title);

/**
 *  Config Form Content
 */
  char resetAPPath[100];
  handlerPath(resetAPPath,100,"resetAP");
  char pathBuff[100];
  getPath(pathBuff,100);
  char svcPath[100];
  setConfiguration()->getPath(svcPath,100);
  pos = formatBuffer_P(buffer,size,pos,ExtendedDevice_config_form,svcPath,getDisplayName(),resetAPPath,pathBuff);

/**
 *  Config Form HTML Tail
 */
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer);   
}

/**
 *   Reset portal to force portal start on next boot cycle, rather than using stored credentials.
 */
void ExtendedDevice::clearAP(WebContext *svr) {
  WiFiPortal::resetCredentials();
  configForm(svr);
}

void ExtendedDevice::resetAP(WebContext *svr) {
  char buffer[1500];
  int size = sizeof(buffer);
  
/**
 *    Config Form HTML Start with Title
 */
  char title[50];
  snprintf(title,50,"Reset Access Point for %s?",getDisplayName());
  int pos = formatHeader(buffer,size,title);

/**
 *  Reset AP Form Content
 *  OK Button calls clearAP()
 *  CANCEL Button returns to the Config Form
 */
  char clearAPPath[100];
  handlerPath(clearAPPath,100,"clearAP");
  char svcPath[100];
  setConfiguration()->getPath(svcPath,100);
  String ssid = WiFi.SSID();
  pos = formatBuffer_P(buffer,size,pos,ExtendedDevice_resetAP,ssid.c_str(),clearAPPath,svcPath);

/**
 *  Config Form HTML Tail
 */
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer);
}

void ExtendedDevice::setup(WebContext* svr) {
  RootDevice::setup(svr);
  char pathBuffer[100];  
  handlerPath(pathBuffer,100,"nearbyDevices");
  svr->on(pathBuffer,[this](WebContext* svr){this->nearbyDevices(svr);});
  pathBuffer[0] = '\0';
  handlerPath(pathBuffer,100,"resetAP");
  svr->on(pathBuffer,[this](WebContext* svr){this->resetAP(svr);});
  pathBuffer[0] = '\0';
  handlerPath(pathBuffer,100,"clearAP");
  svr->on(pathBuffer,[this](WebContext* svr){this->clearAP(svr);});
}

} // End of namespace lsc
