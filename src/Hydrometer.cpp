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

#include "Hydrometer.h"

namespace lsc {

const char Hydrometer_body[]             PROGMEM = "<p align=\"center\" style=\"font-size:1.25em;\"> Soil Moisture: %.1f%% <br> Sensor Reading: %d</p>";
const char Hydrometer_root_body[]        PROGMEM = "<p align=\"center\" style=\"font-size:1.1em;\"> Soil Moisture: %.1f%%</p>";
const char Hydrometer_template[]         PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                          "<Hydrometer>"
                                              "<soilMoisture>%f</soilMoisture>"
                                          "</Hydrometer>";
const char Hydrometer_config_template[]  PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                          "<config>"
                                             "<displayName>%s</displayName>"
                                             "<drySensor>%d</drySensor>"
                                             "<wetSensor>%d</wetSensor>"
                                          "</config>";
const char Hydrometer_config_form[]      PROGMEM = "<form action=\"%s\">"                                                                                 // Service path
            "<div align=\"center\">"
              "<label for=\"displayName\">Sensor Name &nbsp &nbsp </label>"
              "<input type=\"text\" placeholder=\"%s\" name=\"displayName\"><br><br><br>"                                                                 // Display name
              "<H2>Calibration</H2><br>"
              "<label for=\"drySensor\">Dry Sensor &nbsp &nbsp</label>"
              "<input type=\"number\" name=\"drySensor\" min=\"0\" max=\"1000\" align=\"right\" style=\"width:3.1em\" maxlength=\"4\" value=\"%d\">"      // Air      
              "&nbsp &nbsp &nbsp <a style=\"text-decoration:none\" href=\"%s\"><button class=\"fmButton\" type=\"button\">Acquire</button></a><br><br>"   // handler path
              "<label for=\"wetSensor\">Wet Sensor &nbsp </label>"
              "<input type=\"number\" name=\"wetSensor\" min=\"0\" max=\"1000\" align=\"right\" style=\"width:3.1em\" maxlength=\"4\" value=\"%d\">"      // Water
              "&nbsp &nbsp &nbsp <a style=\"text-decoration:none\" href=\"%s\"><button class=\"fmButton\" type=\"button\">Acquire</button></a><br><br>"   // handler path
              "<button class=\"fmButton\" type=\"submit\">Submit</button>&nbsp&nbsp"
              "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button>"                                       // Cancel path
            "</div></form>";


/**
 *  Static RTT initialization
 */
INITIALIZE_SERVICE_TYPES(GetSoilMoisture,LeelanauSoftware-com,getSoilMoisture,1.0.0);
INITIALIZE_DEVICE_TYPES(Hydrometer,LeelanauSoftware-com,Hydrometer,1.0.0);

GetSoilMoisture::GetSoilMoisture() : UPnPService("getSoilMoisture") {setDisplayName("Get Soil Moisture");};
void GetSoilMoisture::handleRequest(WebContext* svr) {
  Hydrometer* h = (Hydrometer*)GET_PARENT_AS(Hydrometer::classType());
  char buffer[256];
  int result = 200;
  if( h != NULL ) {
    float m = h->soilMoisture();
    snprintf_P(buffer,256,Hydrometer_template,m);
  }
  else {
    result = 500;
    snprintf_P(buffer,128,error_html,"Hydrometer");
  }
  svr->send(200,"text/xml",buffer); 
}

Hydrometer::Hydrometer() : Sensor("hydrometer") {
  addServices(&_getSoilMoisture);
  setDisplayName("Hydrometer");
}

Hydrometer::Hydrometer(const char* target) : Sensor(target) {
  addServices(&_getSoilMoisture);
  setDisplayName("Hydrometer");
}

int Hydrometer::pin() {
  return _pin;
}

/** Analog Pin number.
 */
void Hydrometer::pin(int p) {
  if(p >= 0) _pin = p;
}

int Hydrometer::formatContent(char buffer[], int size, int pos) {
  int ar = analogRead(A0);
  float sm = soilMoisture(ar);
  pos = formatBuffer_P(buffer,size,pos,Hydrometer_body,sm,ar);  
  return pos;
}

int Hydrometer::formatRootContent(char buffer[], int size, int pos) {
  float sm = soilMoisture(analogRead(A0));
  pos = formatBuffer_P(buffer,size,pos,Hydrometer_root_body,sm);  
  return pos;
}

float Hydrometer::soilMoisture() {
  int ar = analogRead(A0);
  return soilMoisture(ar);
}

float Hydrometer::soilMoisture(int ar) {
  ar = ((ar<_water)?(_water):((ar>_air)?(_air):(ar)));
  float result = 100.0*(((float)_air - (float)ar)/(float)(_air - _water));
  return result;
}

void Hydrometer::configForm(WebContext* svr) {
/**
 *   Reset acquired values for the form and display with
 *   stored boundaries
 */
  _acquireDry = -1;
  _acquireWet = -1;
  displayForm(svr,air(),water());
}

void Hydrometer::acquireDry(WebContext* svr) {
  int ar = analogRead(A0);
  int dry = ((ar<0)?(0):((ar>1000)?(1000):(ar)));
  int wet = ((_acquireWet<0)?(water()):(_acquireWet));
  _acquireDry = dry;
  displayForm(svr,dry,wet);
}


void Hydrometer::acquireWet(WebContext* svr) {
  int ar = analogRead(A0);
  int dry = ((_acquireDry<0)?(air()):(_acquireDry));
  int wet = ((ar<0)?(0):((ar>1000)?(1000):(ar)));
  _acquireWet = wet;
  displayForm(svr,dry,wet); 
}

/**  Display the config form with boundaries for sensor dry in the air and
 *   sensor wet in water.
 * 
 */
void Hydrometer::displayForm(WebContext* svr, int dry, int wet) {

/** wet and dry could have been set via prior invocation of this form using the acquire buttons.
 *  _acquireWet and _acquireDry must be reinitialized
 */
  char buffer[1500];
  int size = sizeof(buffer);
  
/**
 *    Config Form HTML Start with Title
 */
  int pos = formatHeader(buffer,size,"Set Configuration");

/**
 *  Config Form Content
 */

  char dryPath[100];
  handlerPath(dryPath,100,"acquireDry");
  char wetPath[100];
  handlerPath(wetPath,100,"acquireWet");
  char pathBuff[100];
  getPath(pathBuff,100);
  char svcPath[100];
  setConfigurationSvc()->getPath(svcPath,100);
  pos = formatBuffer_P(buffer,size,pos,Hydrometer_config_form,svcPath,getDisplayName(),dry,dryPath,wet,wetPath,pathBuff);

/**
 *  Config Form HTML Tail
 */
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer); 
}

void Hydrometer::handleSetConfiguration(WebContext* svr) {
  int numArgs = svr->argCount();
  for( int i=0; i<numArgs; i++) {
     const String& argName = svr->argName(i);
     const String& arg = svr->arg(i);
     if( argName.equalsIgnoreCase("DRYSENSOR") ) {
        int val = (int) arg.toInt();
        air(val);
     }
     else if( argName.equalsIgnoreCase("WETSENSOR") ) {
        int val = (int) arg.toInt();
        water(val);
     }
     else if( argName.equalsIgnoreCase("DISPLAYNAME") ) {if( arg.length() > 0 ) setDisplayName(arg.c_str());}
  }
  display(svr);  
}

void Hydrometer::handleGetConfiguration(WebContext* svr) {
  Serial.printf("Hydrometer::getConfiguration: getConfiguration starting...\n");
  char buffer[1000];
  size_t bufferSize = sizeof(buffer);
  int size = bufferSize;
  snprintf_P(buffer,size,Hydrometer_config_template,getDisplayName(),air(),water());
  Serial.printf("Hydrometer::getConfiguration: getConfiguration sending response\n");
  svr->send(200, "text/xml", buffer);     
}

void Hydrometer::setup(WebContext* svr) {
  Sensor::setup(svr);
  char pathBuff[100];
  handlerPath(pathBuff,100,"acquireDry");
  svr->on(pathBuff,[this](WebContext* svr){this->acquireDry(svr);});
  pathBuff[0] = '\0';
  handlerPath(pathBuff,100,"acquireWet");
  svr->on(pathBuff,[this](WebContext* svr){this->acquireWet(svr);});
  pinMode(_pin,INPUT);
}

} // End of namespace lsc
