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

#include "SensorDevice.h"
const char Sensor_config_form[]      PROGMEM = "<form action=\"%s\"><div align=\"center\">"
                                                   "<label for=\"displayName\">Sensor Name &nbsp &nbsp</label>"
                                                   "<input type=\"text\" placeholder=\"%s\" name=\"displayName\"><br><br>"
                                                   "<button class=\"fmButton\" type=\"submit\">Submit</button>&nbsp&nbsp"
                                                   "<a style=\"text-decoration:none\" href=\"/%s\"><button class=\"fmButton\" type=\"button\">Cancel</button></a>"
                                                "</div></form>";
const char Sensor_config_template[]  PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><config><displayName>%s</displayName></config>";

namespace lsc {

INITIALIZE_DEVICE_TYPES(Sensor,LeelanauSoftware-com,Sensor,1.0.0);

Sensor::Sensor() : UPnPDevice("sensor") {
  addServices(getConfigurationSvc(),setConfigurationSvc());   // Add services for configuration
  setDisplayName("Sensor");                                   // Set the eisplay name
  setConfigurationSvc()->setHttpHandler([this](WebContext* svr){this->handleSetConfiguration(svr);});
  setConfigurationSvc()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfigurationSvc()->setHttpHandler([this](WebContext* svr){this->handleGetConfiguration(svr);});
}

Sensor::Sensor(const char* target) : UPnPDevice(target) {
  addServices(getConfigurationSvc(),setConfigurationSvc());   // Add services configuration
  setDisplayName("Sensor");                                   // Set the eisplay name
  setConfigurationSvc()->setHttpHandler([this](WebContext* svr){this->handleSetConfiguration(svr);});
  setConfigurationSvc()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfigurationSvc()->setHttpHandler([this](WebContext* svr){this->handleGetConfiguration(svr);});
}

void Sensor::display(WebContext* svr) {
  char buffer[DISPLAY_SIZE];
  int size = sizeof(buffer);
  int pos = formatHeader(buffer,size,getDisplayName());
  pos = formatContent(buffer,size,pos);
 
/** 
 *  Parent of a Sensor is a RootDevice and thus is non-null and provides a complete path
 *  Add a Config Button to the Sensor display
 */
  char pathBuff[100];
  setConfigurationSvc()->formPath(pathBuff,100);
  pos = formatBuffer_P(buffer,size,pos,config_button,pathBuff,"Configure"); 
   
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer);
}

}
