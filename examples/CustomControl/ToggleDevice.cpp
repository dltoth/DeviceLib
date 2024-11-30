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

#include "ToggleDevice.h"

/**
 * Control Slider ON
 */
const char relay_on[]    PROGMEM = "<div align=\"center\"><a href=\"./setState?STATE=OFF\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\" checked>"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;ON</div>";

/**
 * Control Slider OFF
 */
const char relay_off[]   PROGMEM = "<div align=\"center\">&ensp;<a href=\"./setState?STATE=ON\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\">"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;OFF</div>";

/**
 *  Static RTT and UPnP Type initialization
 */
INITIALIZE_DEVICE_TYPES(ToggleDevice,LeelanauSoftware-com,ToggleDevice,1.0.0);

/**
 *  The only expected arguments are STATE=ON or STATE=OFF, all other arguments are ignored
 */
void ToggleDevice::setState(WebContext* svr) {
   int numArgs = svr->argCount();
   if( numArgs > 0 ) {
      for( int i=0; i<numArgs; i++ ) {
         const String& argName = svr->argName(i);
         const String& argVal = svr->arg(i);
         if(argName.equalsIgnoreCase("STATE")) {
            if( argVal.equalsIgnoreCase("ON")) setState(ON);
            else if( argVal.equalsIgnoreCase("OFF") ) setState(OFF);
            break;
          }
       }
   }

/** Control refresh is only within the iFrame
 */
   displayControl(svr);
}

int  ToggleDevice::formatContent(char buffer[], int size, int pos) { 
  if( isON() ) {pos = formatBuffer_P(buffer,size,pos,relay_on);}        
  else {pos = formatBuffer_P(buffer,size,pos,relay_off);} 
  return pos;        
}

void ToggleDevice::setup(WebContext* svr) {
   Control::setup(svr);
   char pathBuffer[100];
   handlerPath(pathBuffer,100,"setState");
   svr->on(pathBuffer,[this](WebContext* svr){this->setState(svr);});  
}
