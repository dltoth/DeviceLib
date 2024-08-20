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

#include "Control.h"

namespace lsc {
/**
 *  Static initialization for RTT and UPnP device type
 */
INITIALIZE_DEVICE_TYPES(Control,LeelanauSoftware-com,Control,1.0.0);

Control::Control() : UPnPDevice("control") {
  addServices(getConfigurationSvc(),setConfigurationSvc());   // Add services for configuration
  setDisplayName("Control");
  setConfigurationSvc()->setHttpHandler([this](WebContext* svr){this->handleSetConfiguration(svr);});
  setConfigurationSvc()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfigurationSvc()->setHttpHandler([this](WebContext* svr){this->handleGetConfiguration(svr);});
}

Control::Control(const char* target) : UPnPDevice(target) {
  addServices(getConfigurationSvc(),setConfigurationSvc());   // Add services for configuration
  setDisplayName("Control");
  setConfigurationSvc()->setHttpHandler([this](WebContext* svr){this->handleSetConfiguration(svr);});
  setConfigurationSvc()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfigurationSvc()->setHttpHandler([this](WebContext* svr){this->handleGetConfiguration(svr);});
}

/**
 *  Root content display is an iFrame with miniture (L3) title
 */
int Control::formatRootContent(char buffer[], int size, int pos) {
  pos = formatBuffer_P(buffer,size,pos,html_L3_title,getDisplayName());
  char pathBuff[100];
  contentPath(pathBuff,100);
  pos = formatBuffer_P(buffer,size,pos,iframe_html,pathBuff,frameHeight(),frameWidth());
  return pos;     
}
/**
 *  Display iFrame with title decoration
 */
void Control::display(WebContext* svr) {
  char buffer[500];
  int size = sizeof(buffer);
  int pos = formatHeader(buffer,size,getDisplayName());
  char pathBuff[100];
  contentPath(pathBuff,100);
  
/**
 *   iFrame display takes url, height, and width as arguments
 */
  pos = formatBuffer_P(buffer,size,pos,iframe_html,pathBuff,frameHeight(),frameWidth());

/** 
 *  Add a Config Button to the Control display
 */
  setConfigurationSvc()->formPath(pathBuff,100);
  pos = formatBuffer_P(buffer,size,pos,config_button,pathBuff,"Configure"); 
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer);
}

/**
 *   Display iFrame content only, no title decoration
 */
void Control::displayControl(WebContext* svr) {
  char buffer[DISPLAY_SIZE];
  int size = sizeof(buffer);
  int pos = 0;
  pos = formatBuffer_P(buffer,size,pos,html_header);
  pos = formatContent(buffer,size,pos);
  pos = formatTail(buffer,size-pos,pos); 
  svr->send(200,"text/html",buffer);
}

void Control::setup(WebContext* svr) {
  UPnPDevice::setup(svr);
  char pathBuff[100];
  contentPath(pathBuff,100);
  svr->on(pathBuff,[this](WebContext* svr){this->displayControl(svr);});
}

void Control::contentPath(char buffer[], size_t size) {handlerPath(buffer,size,"displayControl");}

} // End of namespace lsc
