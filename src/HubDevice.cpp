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
 
#include "HubDevice.h"
#include <UPnPBuffer.h>

namespace lsc {

const char SSDP_RootSearch[]            PROGMEM = "M-SEARCH * HTTP/1.1\r\n"
                                                  "HOST: 239.255.255.250:1900\r\n"
                                                  "MAN: ssdp:discover\r\n"
                                                  "ST: upnp:rootdevice\r\n"
                                                  "ST.LEELANAUSOFTWARE.COM: \r\n"
                                                  "USER-AGENT: ESP8266 UPnP/1.1 LSC-SSDP/1.0\r\n\r\n";
const char LocationHeader[]             PROGMEM = "LOCATION";
const char display_html[]               PROGMEM = "<H3 align=\"center\">Devices on %s</H3><br>";
const char brk_html[]                   PROGMEM = "<br><brk>";   
   
#define SSDP_BUFFER_SIZE 1000
#define DESC_HEADER_SIZE 150

/**
 *  Static RTT initialization
 */
 
INITIALIZE_STATIC_TYPE(HubDevice);
INITIALIZE_UPnP_TYPE(HubDevice,urn:LeelanauSoftware-com:device:HubDevice:1);

HubDevice::HubDevice() : ExtendedDevice("hub") {
  setDisplayName("Device Hub");
}

HubDevice::HubDevice(const char* target) : ExtendedDevice(target) {
  setDisplayName("Device Hub");
}

void HubDevice::displayRoot(WebContext* svr) {
  
  char buffer[DISPLAY_SIZE];
  int size = sizeof(buffer);
      
/** Add HTML Header and Title with Display Name
 */
  int pos = formatHeader(buffer,size,getDisplayName());
  pos = formatBuffer_P(buffer,size,pos,display_html,WiFi.SSID().c_str());

/** 
 *  Search for nearby RootDevices. As each one is parsed (content filled into a UPnPBuffer), create a button with link
 *  to location of the RootDevice.
 */
  char* buff = buffer;
  SSDP::searchRequest("upnp:rootdevice",([buff,size,&pos](UPnPBuffer* b){
       char name[32];
       if( b->displayName(name,32) ) {
           char loc[128];
/**
 *         If a LOCATION header is present on the Search Reply, copy it to the loc buffer then add an app_button with location.
 *         Otherwise add an app_button with "/" as location (which just redirects to the display).
 */
           if( b->headerValue_P(LocationHeader,loc,128) ) pos = formatBuffer_P(buff,size,pos,app_button,loc,name);
           else pos = formatBuffer_P(buff,size,pos,app_button,"/",name);     
       }  
    }),WiFi.localIP());

  pos = formatBuffer_P(buffer,size,pos,brk_html);
  formatContent(buffer+pos,size-pos);
  pos = strlen(buffer);

/** Add the HTML tail
 */ 
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer);
  Serial.printf("Sending %d bytes of %d\n",strlen(buffer),DISPLAY_SIZE);
}

} // End of namespace lsc
