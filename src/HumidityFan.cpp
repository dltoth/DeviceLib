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

#include "HumidityFan.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

const char threshold_setting[] PROGMEM = "<br><div align=\"center\">Threshold Set To %d%%</div>";
const char humidity_display[]  PROGMEM = "<br><div align=\"center\">Humidity is %.1f%%</div>";

/**
 *    Variable input to form is service action url, display name placeholder, threshold placeholder, cancel url
 */
const char HumidityFan_config_form[]     PROGMEM = "<form action=\"%s\"><div align=\"center\">"
                                         "<label for=\"displayName\">Sensor Name:</label>&emsp;"
                                         "<input type=\"text\" placeholder=\"%s\" name=\"displayName\">&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;<br><br>"
                                         "<label for=\"threshold\">Humidity Threshold:</label>&emsp;"
                                         "<input type=\"text\" id=\"threshold\" placeholder=\"%d\" name=\"threshold\" pattern=\"[0-9]{2}\">&ensp;(between 0 and 99)<br><br>"
                                         "<div align=\"center\">Humidity is %.1f%%</div><br>"
                                         "<button class=\"fmButton\" type=\"submit\">Submit</button>&nbsp&nbsp"
                                         "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button>" 
                                         "</div></form>";
                                      
const char HumidityFan_config_template[]  PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                                      "<config>"
                                                         "<displayName>%s</displayName>"
                                                         "<threshold>%d</threshold>"
                                                      "</config>";

/**
 *  Static RTT initialization
 */
INITIALIZE_STATIC_TYPE(HumidityFan);
INITIALIZE_UPnP_TYPE(HumidityFan,urn:LeelanauSoftware-com:device:HumidityFan:1);

HumidityFan::HumidityFan() : SensorControlledRelay("humidityFan") {
  setDisplayName("Humidity Fan");
  
/**
 *   Setup Configuration handlers
 */
  setConfiguration()->setHttpHandler([this](WebContext* svr){this->setHumidityFanConfiguration(svr);});
  setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfiguration()->setHttpHandler([this](WebContext* svr){this->getHumidityFanConfiguration(svr);});
}

HumidityFan::HumidityFan(const char* target) : SensorControlledRelay(target) {
  setDisplayName("Humidity Fan");

/**
 *   Setup Configuration handlers
 */
  setConfiguration()->setHttpHandler([this](WebContext* svr){this->setHumidityFanConfiguration(svr);});
  setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfiguration()->setHttpHandler([this](WebContext* svr){this->getHumidityFanConfiguration(svr);});
}

void  HumidityFan::content(char buffer[], int size) {  
  SensorControlledRelay::content(buffer,size);
  int pos = strlen(buffer);  
//  pos = formatBuffer_P(buffer,size,pos,humidity_display,humidity());         
  if( isAUTOMATIC() )  pos = formatBuffer_P(buffer,size,pos,threshold_setting,threshold());       
}

/**
 *   Return ControlState as humidity measured against threshold.
 *   Sensor readings must vary from threshold by more than 1% to change the outcome.
 */
ControlState HumidityFan::sensorState() {
  ControlState result = lastSensorState();
  Thermometer* t = getThermometer();
  if ( t != NULL ) humidity(t->hum());
  float delta = humidity() - (float)threshold();
  if( delta > 0.5 ) result = ON;
  else if( delta < -0.5 ) result = OFF;
  return result;
}

void HumidityFan::configForm(WebContext* svr) {
  char buffer[1000];
  int size = sizeof(buffer);
  
/**
 *    Config Form HTML Start with Title
 */
  int pos = formatHeader(buffer,size,"Control Configuration");

/**
 *  Config Form Content
 */ 
  char pathBuff[100];
  getPath(pathBuff,100);
  char svcPath[100];
  setConfiguration()->getPath(svcPath,100);
  
  pos = formatBuffer_P(buffer,size,pos,HumidityFan_config_form,svcPath,getDisplayName(),threshold(),humidity(),pathBuff);

/**
 *  Config Form HTML Tail
 */
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer); 
}

void HumidityFan::setHumidityFanConfiguration(WebContext* svr) {
  int numArgs = svr->argCount();
  for( int i=0; i<numArgs; i++) {
     const String& argName = svr->argName(i);
     const String& arg = svr->arg(i);
     if( argName.equalsIgnoreCase("THRESHOLD") ) {
        int t = (int) arg.toInt();
        if( t > 0 ) threshold(t);
     }
     else if( argName.equalsIgnoreCase("DISPLAYNAME") ) {if( arg.length() > 0 ) setDisplayName(arg.c_str());}
  }
  display(svr);  
}

void HumidityFan::getHumidityFanConfiguration(WebContext* svr) {
  char buffer[1000];
  size_t bufferSize = sizeof(buffer);
  int size = bufferSize;
  snprintf_P(buffer,size,HumidityFan_config_template,getDisplayName(),threshold());
  svr->send(200, "text/xml", buffer);     
}

} // End of namespace lsc