/**
 * 
 */

#include "SensorControlledRelay.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

const char msg_display[] PROGMEM = "<br><div align=\"center\">%s</div>";

/**
 * Relay Slider ON
 */
const char state_on[]    PROGMEM = "<div align=\"center\"><a href=\"./setState?STATE=OFF\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\" checked>"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;ON&nbsp;</div>";

/**
 * Relay Slider OFF
 */
const char state_off[]   PROGMEM = "<div align=\"center\"><a href=\"./setState?STATE=ON\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\">"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;OFF</div>";

/**
 * Automatic Slider ON
 */
const char mode_auto[]   PROGMEM = "<div align=\"center\">&emsp;&emsp;&nbsp;<a href=\"./setMode?MODE=MANUAL\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\" checked>"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Automatic</div><br>";

/**
 * Automatic Slider OFF
 */
const char mode_manual[] PROGMEM = "<div align=\"center\">&emsp;&emsp;<a href=\"./setMode?MODE=AUTOMATIC\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\">"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Manual&nbsp;&nbsp;&nbsp;</div><br>";

/**
 *  Static RTT initialization
 */
INITIALIZE_STATIC_TYPE(SensorControlledRelay);

SensorControlledRelay::SensorControlledRelay() : RelayControl("urn:LeelanauSoftwareCo-com:device:SensorControlledRelay:1","SensorControlledRelay"),
                                 _setModeSvc("urn:LeelanauSoftwareCo-com:service:setMode:1","setMode") {
  addService(setModeSvc());
  setModeSvc()->setHttpHandler([this](WebContext* svr){this->setMode(svr);});
  setDisplayName("Sensor Controlled Relay");
  sensorRefresh(SENSOR_REFRESH);
  _timer.setHandler([this]{this->timerCallback();});
}

SensorControlledRelay::SensorControlledRelay(const char* type, const char* target) : RelayControl(type, target),
                                 _setModeSvc("urn:LeelanauSoftwareCo-com:service:setMode:1","setMode") {
  addService(setModeSvc());
  setModeSvc()->setHttpHandler([this](WebContext* svr){this->setMode(svr);});
  setDisplayName("Sensor Controlled Relay");
  sensorRefresh(SENSOR_REFRESH);
  _timer.setHandler([this]{this->timerCallback();});
}

void  SensorControlledRelay::content(char buffer[], int size) {  
  int pos = 0;
  if( loggingLevel(FINE) ) Serial.printf("SensorControlledRelay::content: Relay state is %s and mode is %s\n",controlState(),controlMode());
  if( isAUTOMATIC() ) pos = formatBuffer_P(buffer,size,pos,mode_auto);
  else pos = formatBuffer_P(buffer,size,pos,mode_manual);
  if( isON() ) pos = formatBuffer_P(buffer,size,pos,state_on);         
  else pos = formatBuffer_P(buffer,size,pos,state_off);          
}

/**
 *   Called by RelayControl when the state toggle is triggered, as implemented in the set state service. In our case we want 
 *   to change mode to MANUAL as well.
 */
void SensorControlledRelay::setControlState(ControlState flag) {
   relayState(flag);
   if(loggingLevel(FINE)) Serial.printf("SensorControlledRelay::setControlState setting state to %s\n",controlState());
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