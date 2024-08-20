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

#include "SensorControlledRelay.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

const char msg_display[]     PROGMEM = "<br><div align=\"center\">%s</div>";

const char table_start[]     PROGMEM = "<div align=\"center\"><table>";

// Control State = ON
const char table_state_ON[]  PROGMEM = "<tr>"
        "<td><a href=\"./setState?STATE=OFF\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\" checked><span class=\"toggle-switch\"></span></a></td>"
        "<td>&ensp;ON</td>"
      "</tr>";

// Control State = OFF
const char table_state_OFF[] PROGMEM = "<tr>"
        "<td><a href=\"./setState?STATE=ON\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\"><span class=\"toggle-switch\"></span></a></td>"
        "<td>&ensp;OFF</td>"
      "</tr>";

// Control Mode = AUTOMATIC
const char table_mode_AUTO[]  PROGMEM = "<tr>"
        "<td><a href=\"./setMode?MODE=MANUAL\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\" checked><span class=\"toggle-switch\"></span></a></td>"
        "<td>&ensp;Automatic</td>"
      "</tr>";

// Control Mode = MANUAL
const char table_mode_MAN[] PROGMEM = "<tr>"
        "<td><a href=\"./setMode?MODE=AUTOMATIC\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\"><span class=\"toggle-switch\"></span></a></td>"
        "<td>&ensp;Manual&ensp;&ensp;</td>"
      "</tr>";
    
const char table_tail[]      PROGMEM = "</table><br></div>";

/**
 * Relay Slider ON

const char state_on[]    PROGMEM = "<div align=\"center\"><a href=\"./setState?STATE=OFF\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\" checked>"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;ON</div>";

 * Relay Slider OFF

const char state_off[]   PROGMEM = "<div align=\"center\"><a href=\"./setState?STATE=ON\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\">"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;OFF</div>";

 * Automatic Slider ON

const char mode_auto[]   PROGMEM = "<div align=\"center\">&emsp;&emsp;&nbsp;<a href=\"./setMode?MODE=MANUAL\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\" checked>"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Automatic</div><br>";


 * Automatic Slider OFF

const char mode_manual[] PROGMEM = "<div align=\"center\">&emsp;&emsp;<a href=\"./setMode?MODE=AUTOMATIC\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\">"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Manual</div><br>";
**/
/**
 *  Static RTT and UPnP type initialization
 */
INITIALIZE_DEVICE_TYPES(SensorControlledRelay,LeelanauSoftware-com,SensorControlledRelay,1.0.0);

SensorControlledRelay::SensorControlledRelay() : RelayControl("SensorControlledRelay"), _setModeSvc("setMode") {
  
  addService(setModeSvc());
  setModeSvc()->setHttpHandler([this](WebContext* svr){this->setMode(svr);});
  setDisplayName("Sensor Controlled Relay");
  sensorRefresh(SENSOR_REFRESH);
  _timer.setHandler([this]{this->timerCallback();});
}

SensorControlledRelay::SensorControlledRelay(const char* target) : RelayControl(target), _setModeSvc("setMode") {
  addService(setModeSvc());
  setModeSvc()->setHttpHandler([this](WebContext* svr){this->setMode(svr);});
  setDisplayName("Sensor Controlled Relay");
  sensorRefresh(SENSOR_REFRESH);
  _timer.setHandler([this]{this->timerCallback();});
}

int  SensorControlledRelay::formatContent(char buffer[], int size, int pos) {  
  if( loggingLevel(FINE) ) Serial.printf("SensorControlledRelay::formatContent: %s Relay state is %s and mode is %s\n",getDisplayName(),controlState(),controlMode());
  pos = formatBuffer_P(buffer,size,pos,table_start);
  if( isAUTOMATIC() ) pos = formatBuffer_P(buffer,size,pos,table_mode_AUTO);
  else pos = formatBuffer_P(buffer,size,pos,table_mode_MAN);
  if( isON() ) pos = formatBuffer_P(buffer,size,pos,table_state_ON);         
  else pos = formatBuffer_P(buffer,size,pos,table_state_OFF);  
  pos = formatBuffer_P(buffer,size,pos,table_tail);
  return pos;        
}

/**
 *   Called by RelayControl when the state toggle is triggered, as implemented in the set state service. In our case we want 
 *   to change mode to MANUAL as well.
 */
void SensorControlledRelay::setControlState(ControlState flag) {
   if(loggingLevel(FINE)) Serial.printf("SensorControlledRelay::setControlState: Relay state is %s, setting state to %s\n",controlState(),((flag==ON)?("ON"):("OFF")));
   relayState(flag);
   if(isAUTOMATIC()) {
     if(loggingLevel(FINE)) Serial.printf("SensorControlledRelay::setControlState setting mode to MANUAL\n");
     setControlMode(MANUAL);
   }
}

/**
 *   Set ControlMode when the mode toggle is triggered. If mode is flipped from MANUAL to AUTOMATIC, 
 *   then the state may also need to change.
 */
void SensorControlledRelay::setControlMode(ControlMode flag) {
   _mode = flag;
   if( loggingLevel(FINE) ) Serial.printf("SensorControlledRelay::setControlMode ControlMode set to %s\n",controlMode());
   if(isAUTOMATIC()) {
     ControlState state = sensorState();
     if( relayState() != state ) {
       if( loggingLevel(FINE) ) Serial.printf("SensorControlledRelay::setControlMode: resetting controlState to %s\n",controlState());
       relayState(state);
     }
   }
}

/**
 *  The only expected arguments are MODE=AUTOMATIC or MOD=MANUAL, all other arguments are ignored
 */
void SensorControlledRelay::setMode(WebContext* svr) {
   int numArgs = svr->argCount();
   if( numArgs > 0 ) {
      for( int i=0; i<numArgs; i++ ) {
         const String& argName = svr->argName(i);
         const String& argVal = svr->arg(i);
         if(argName.equalsIgnoreCase("MODE")) {
            if( loggingLevel(FINE) ) Serial.printf("SensorControlledRelay::setMode: Setting MODE to %s\n",argVal.c_str());
            if( argVal.equalsIgnoreCase("AUTOMATIC")) setControlMode(AUTOMATIC);
            else if( argVal.equalsIgnoreCase("MANUAL") ) setControlMode(MANUAL);
            break;
          }
       }
   }
   displayControl(svr);
}

void SensorControlledRelay::setup(WebContext* svr) {
  RelayControl::setup(svr);
  lastSensorState(getControlState());
  _timer.start();
}

void SensorControlledRelay::doDevice() {
  _timer.doDevice();
}

} // End of namespace lsc