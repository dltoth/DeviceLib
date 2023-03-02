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

#include "Thermometer.h"

namespace lsc {

const char TempHum_body[]                PROGMEM = "<p align=\"center\"> Temperature: %.1f %c<br> Humidity: %.1f%% </p>";
const char TempHum_template[]            PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><TempHum>"
                                                      "<temp>%f %c</temp>"
                                                      "<hum>%f</hum>"
                                                   "</TempHum>";
const char Thermometer_config_template[]  PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                            "<config>"
                                               "<displayName>%s</displayName>"
                                               "<unit>%c</unit>"
                                            "</config>";
const char Thermometer_config_form[]      PROGMEM = "<form action=\"%s\"><div align=\"center\">"
            "<label for=\"displayName\">Sensor Name &nbsp &nbsp</label>"
            "<input type=\"text\" placeholder=\"%s\" name=\"displayName\"><br><br>"
            "<label for=\"unit\">Thermometer Unit &nbsp </label>"
            "<input type=\"text\" name=\"unit\" pattern=\"[FC]{1}\" style=\"width:2.1em;font-size:1em\" maxlength=\"1\" placeholder=\"%c\"><br><br>"        
            "<button class=\"fmButton\" type=\"submit\">Submit</button>&nbsp&nbsp"
            "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button>"  // Cancel path
            "</div></form>";

/**
 *  Static RTT initialization
 */
INITIALIZE_STATIC_TYPE(GetTempHum);
INITIALIZE_UPnP_TYPE(GetTempHum,urn:LeelanauSoftware-com:service:getTempHum:1);
INITIALIZE_STATIC_TYPE(Thermometer);
INITIALIZE_UPnP_TYPE(Thermometer,urn:LeelanauSoftware-com:device:Thermometer:1);

GetTempHum::GetTempHum() : UPnPService("getTempHum") {setDisplayName("Get Temperature/Humidity");};
void GetTempHum::handleRequest(WebContext* svr) {
  Thermometer* t = (Thermometer*)GET_PARENT_AS(Thermometer::classType());
  char buffer[256];
  int result = 200;
  if( t != NULL ) {
    float temp = t->temp();
    float hum  = t->hum();
    snprintf_P(buffer,256,TempHum_template,temp,t->unit(),hum);
  }
  else {
    result = 500;
    snprintf_P(buffer,128,error_html,"Thermometer");
  }
  svr->send(200,"text/xml",buffer); 
}

Thermometer::Thermometer() : Sensor("thermometer") {
  addServices(&_getTempHum);
  setDisplayName("Thermometer");
  setConfiguration()->setHttpHandler([this](WebContext* svr){this->setThermometerConfiguration(svr);});
  setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfiguration()->setHttpHandler([this](WebContext* svr){this->getThermometerConfiguration(svr);});
}

Thermometer::Thermometer(const char* target) : Sensor(target) {
  addServices(&_getTempHum);
  setDisplayName("Thermometer");
  setConfiguration()->setHttpHandler([this](WebContext* svr){this->setThermometerConfiguration(svr);});
  setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfiguration()->setHttpHandler([this](WebContext* svr){this->getThermometerConfiguration(svr);});}

float Thermometer::temp() {
  float result = _dht.getTemperature();
  if( isFahrenheit() ) result = _dht.toFahrenheit(result);
  return result; 
}

float Thermometer::hum() {
  float result = _dht.getHumidity();
  return result; 
}

int Thermometer::pin() {
  return _pin;
}

/** Digital Pin number.
 *  Must be between 2 and 8 inclusive
 */
void Thermometer::pin(int p) {
  _pin = ((p>1)?((p<9)?(p):(D2)):(D2));
}

char  Thermometer::unit() {return _unit;}
void  Thermometer::setFahrenheit() {_unit = FAHRENHEIT;}
void  Thermometer::setCelcius() {_unit = CELCIUS;}

void Thermometer::content(char buffer[], int bufferSize) {
  float t = temp();
  float h = hum();
  snprintf_P(buffer,bufferSize,TempHum_body,t,unit(),h);  
}

void Thermometer::configForm(WebContext* svr) {
  char buffer[1000];
  int size = sizeof(buffer);
  
/**
 *    Config Form HTML Start with Title
 */
  int pos = formatHeader(buffer,size,"Thermometer Configuration");

/**
 *  Config Form Content
 */
  char pathBuff[100];
  getPath(pathBuff,100);
  char svcPath[100];
  setConfiguration()->getPath(svcPath,100);
  pos = formatBuffer_P(buffer,size,pos,Thermometer_config_form,svcPath,getDisplayName(),unit(),pathBuff);

/**
 *  Config Form HTML Tail
 */
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer); 
}

void Thermometer::setThermometerConfiguration(WebContext* svr) {
  int numArgs = svr->argCount();
  for( int i=0; i<numArgs; i++) {
     const String& argName = svr->argName(i);
     const String& arg = svr->arg(i);
     if( argName.equalsIgnoreCase("UNIT") ) {
        if(isFahrenheit(arg.c_str()[0])) setFahrenheit();
        else if(isCelcius(arg.c_str()[0])) setCelcius();
     }
     else if( argName.equalsIgnoreCase("DISPLAYNAME") ) {if( arg.length() > 0 ) setDisplayName(arg.c_str());}
  }
  display(svr);  
}

void Thermometer::getThermometerConfiguration(WebContext* svr) {
  char buffer[1000];
  size_t bufferSize = sizeof(buffer);
  int size = bufferSize;
  snprintf_P(buffer,size,Thermometer_config_template,getDisplayName(),unit());
  svr->send(200, "text/xml", buffer);     
}

void Thermometer::setup(WebContext* svr) {
  Sensor::setup(svr);
  _dht.setup(pin(), DHTesp::DHT22);
}

} // End of namespace lsc
