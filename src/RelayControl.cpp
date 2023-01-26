/**
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
 * Manual Slider OFF
 */
const char relay_off[]  PROGMEM = "<div align=\"center\">&ensp;<a href=\"./setState?STATE=ON\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\">"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Relay OFF</div>";

/**
 *  Static RTT initialization
 */
INITIALIZE_STATIC_TYPE(RelayControl);

RelayControl::RelayControl() : Control("urn:LeelanauSoftwareCo-com:device:RelayControl:1","RelayControl"),
                                 _setStateSvc("urn:LeelanauSoftwareCo-com:service:setState:1","setState") {
  addService(setStateSvc());
  setStateSvc()->setHttpHandler([this](WebContext* svr){this->setState(svr);});
  setDisplayName("Relay Control");
}

RelayControl::RelayControl(const char* type, const char* target) : Control(type, target),
                                 _setStateSvc("urn:LeelanauSoftwareCo-com:service:setState:1","setState") {
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
   content(svr);
}

void  RelayControl::content(WebContext* svr) {  
  char buffer[1000];
  int size = sizeof(buffer);
  int pos = 0;
  pos = formatBuffer_P(buffer,size,pos,html_header);
  if( loggingLevel(FINE) ) Serial.printf("RelayControl::content: Relay state is %s \n",controlState());
  if( isON() ) {
    pos = formatBuffer_P(buffer,size,pos,relay_on);  
    pos = formatBuffer_P(buffer,size,pos,on_msg);          
  }        
  else {
    pos = formatBuffer_P(buffer,size,pos,relay_off); 
    pos = formatBuffer_P(buffer,size,pos,off_msg);          
  }         
  pos = formatTail(buffer,size,pos); 
  if( loggingLevel(FINE) ) Serial.printf("RelayControl::content: Sending %d bytes\n\n",pos);                             
  svr->send(200,"text/html",buffer);
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
