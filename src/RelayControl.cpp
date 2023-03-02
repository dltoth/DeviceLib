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

#include "RelayControl.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

const char on_msg[]     PROGMEM = "<br><div align=\"center\">Relay is ON</div>";
const char off_msg[]    PROGMEM = "<br><div align=\"center\">Relay is OFF</div>";

/**
 * Relay Slider ON
 */
const char relay_on[]   PROGMEM = "<div align=\"center\"><a href=\"./setState?STATE=OFF\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\" checked>"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Relay ON</div>";

/**
 * Relay Slider OFF
 */
const char relay_off[]  PROGMEM = "<div align=\"center\">&ensp;<a href=\"./setState?STATE=ON\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\">"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Relay OFF</div>";

/**
 *  Static RTT initialization
 */
INITIALIZE_STATIC_TYPE(RelayControl);
INITIALIZE_UPnP_TYPE(RelayControl,urn:LeelanauSoftware-com:device:RelayControl:1);

RelayControl::RelayControl() : Control("RelayControl"), _setStateSvc("setState") {
  addService(setStateSvc());
  setStateSvc()->setHttpHandler([this](WebContext* svr){this->setState(svr);});
  setDisplayName("Relay Control");
}

RelayControl::RelayControl(const char* target) : Control(target), _setStateSvc("setState") {
  addService(setStateSvc());
  setStateSvc()->setHttpHandler([this](WebContext* svr){this->setState(svr);});
  setDisplayName("Relay Control");
}

/**
 *  The only expected arguments are STATE=ON or STATE=OFF, all other arguments are ignored
 */
void RelayControl::setState(WebContext* svr) {
   int numArgs = svr->argCount();
   if( numArgs > 0 ) {
      for( int i=0; i<numArgs; i++ ) {
         const String& argName = svr->argName(i);
         const String& argVal = svr->arg(i);
         if(argName.equalsIgnoreCase("STATE")) {
            if( loggingLevel(FINE) ) Serial.printf("RelayControl::setMode: Setting STATE to %s\n",argVal.c_str());
            if( argVal.equalsIgnoreCase("ON")) setControlState(ON);
            else if( argVal.equalsIgnoreCase("OFF") ) setControlState(OFF);
            break;
          }
       }
   }
   displayControl(svr);
}

void  RelayControl::content(char buffer[], int size) {  
  int pos = 0;
  if( loggingLevel(FINE) ) Serial.printf("RelayControl::content: Relay state is %s \n",controlState());
  if( isON() ) {
    pos = formatBuffer_P(buffer,size,pos,relay_on);  
    pos = formatBuffer_P(buffer,size,pos,on_msg);          
  }        
  else {
    pos = formatBuffer_P(buffer,size,pos,relay_off); 
    pos = formatBuffer_P(buffer,size,pos,off_msg);          
  }         
}


void RelayControl::setControlState(ControlState flag) {
  
/**
 *  If ControlState is set to ON, send HIGH to the relay
 */
  if(flag == ON) {
    digitalWrite(getPin(),HIGH);
    if( loggingLevel(FINE) ) Serial.printf("RelayControl::setControlState: Outlet turned ON, set to %d\n", HIGH);
  }
/**
 *  Otherwise send LOW
 */
  else {
    digitalWrite(getPin(),LOW);
    if( loggingLevel(FINE) ) Serial.printf("RelayControl::setControlState: Outlet turned OFF, set to %d\n", LOW);
  }
}

void RelayControl::setup(WebContext* svr) {
  Control::setup(svr);
  pinMode(getPin(),OUTPUT);
  digitalWrite(getPin(),LOW);
  if( loggingLevel(FINE) ) Serial.printf("RelayControl::setup: Digital pin %d initialized to %d and set to %d\n",getPin(),OUTPUT,LOW);
}

} // End of namespace lsc
