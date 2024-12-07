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
         "<button class=\"fmButton\" type=\"submit\">Submit</button>&nbsp&nbsp"
         "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button>"                                           // Cancel path
      "</div></form><br><br>";

const char brk_html[]                   PROGMEM = "<br>";   
#define SSDP_BUFFER_SIZE 1000
#define DESC_HEADER_SIZE 150

/**
 *  Static RTT initialization
 */
 INITIALIZE_DEVICE_TYPES(ExtendedDevice,LeelanauSoftware-com,ExtendedDevice,1.0.1);

ExtendedDevice::ExtendedDevice() : RootDevice("root") {
  addServices(getConfigurationSvc(),setConfigurationSvc());   // Add services for configuration
  setDisplayName("Extended Device");
  setConfigurationSvc()->setHttpHandler([this](WebContext* svr){this->handleSetConfiguration(svr);});
  setConfigurationSvc()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfigurationSvc()->setHttpHandler([this](WebContext* svr){this->handleGetConfiguration(svr);});
}

ExtendedDevice::ExtendedDevice(const char* target) : RootDevice(target) {
  addServices(getConfigurationSvc(),setConfigurationSvc());   // Add services for configuration
  setDisplayName("Extended Device");
  setConfigurationSvc()->setHttpHandler([this](WebContext* svr){this->handleSetConfiguration(svr);});
  setConfigurationSvc()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfigurationSvc()->setHttpHandler([this](WebContext* svr){this->handleGetConfiguration(svr);});
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
  pos = formatContent(buffer,size,pos);

/** Add a Config button 
 */
  char pathBuff[100];
  setConfigurationSvc()->formPath(pathBuff,100);
  pos = formatBuffer_P(buffer,size,pos,brk_html);
  pos = formatBuffer_P(buffer,size,pos,config_button,pathBuff,"Configure");

/** Add the HTML tail
 */ 
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer);
}

void ExtendedDevice::displayRoot(WebContext* svr) {
  char buffer[DISPLAY_SIZE];
  int size = sizeof(buffer);

/** Add HTML Header and Title with Display Name
 */
  int pos = formatHeader(buffer,size,getDisplayName());

/** Add Content 
 */
  pos = formatRootContent(buffer,size,pos);

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

  IPAddress remote = svr->client().remoteIP();
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
  char pathBuff[100];
  getPath(pathBuff,100);
  char svcPath[100];
  setConfigurationSvc()->getPath(svcPath,100);
  pos = formatBuffer_P(buffer,size,pos,ExtendedDevice_config_form,svcPath,getDisplayName(),pathBuff);

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
}

} // End of namespace lsc
