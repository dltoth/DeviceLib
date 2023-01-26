/**
 * 
 */

#include "FanControl.h"

namespace lsc {

const char fan_on[]            PROGMEM = "<br><div align=\"center\">Fan is ON</div>";
const char fan_off[]           PROGMEM = "<br><div align=\"center\">Fan is OFF</div>";
const char threshold_setting[] PROGMEM = "<br><div align=\"center\">Threshold Set To %d%%</div>";
const char humidity_display[]  PROGMEM = "<br><div align=\"center\">Humidity is %.1f%%</div><br><br>";

/**
 * Automatic Slider ON
 */
const char auto_on[]     PROGMEM = "<div align=\"center\"><a href=\"./setMode?MODE=MANUAL\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\" checked>"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Automatic&nbsp;</div>";

/**
 * Automatic Slider OFF
 */
const char auto_off[]    PROGMEM = "<div align=\"center\"><a href=\"./setMode?MODE=AUTOMATIC\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\">"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Manual&nbsp;</div><br>";

/**
 * Manual Slider ON
 */
const char manual_on[]   PROGMEM = "<div align=\"center\"><a href=\"./setMode?MODE=MANUAL&STATE=OFF\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\" checked>"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Fan ON</div>";

/**
 * Manual Slider OFF
 */
const char manual_off[]  PROGMEM = "<div align=\"center\">&ensp;<a href=\"./setMode?MODE=MANUAL&STATE=ON\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\">"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Fan OFF</div>";


/**
 *    Variable input to form is service action path, display name placeholder, threshold placeholder, cancel path
 */
const char FanControl_config_form[]  PROGMEM = "<form action=\"%s\"><div align=\"center\">"
                                      "<label for=\"displayName\">Sensor Name:</label>&emsp;"
                                      "<input type=\"text\" placeholder=\"%s\" name=\"displayName\">&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;<br><br>"
                                      "<label for=\"humidity\">Humidity Threshold:</label>&emsp;"
                                      "<input type=\"text\" id=\"humidity\" placeholder=\"%d\" name=\"humidity\" pattern=\"[0-9]{2}\">&ensp;(between 0 and 99)<br><br><br>"
                                      "<button class=\"fmButton\" type=\"submit\">Submit</button>&nbsp&nbsp"
                                      "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button>" 
                                      "</div></form>";
                                      
const char FanControl_config_template[]  PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                                      "<config>"
                                                         "<displayName>%s</displayName>"
                                                         "<humidity>%d</humidity>"
                                                      "</config>";

/**
 *  Static RTT initialization
 */
INITIALIZE_STATIC_TYPE(FanControl);

FanControl::FanControl() : Control("urn:LeelanauSoftwareCo-com:device:FanControl:1","FanControl"),_setModeSvc("urn:LeelanauSoftwareCo-com:service:setMode:1","setMode") {
  addService(setModeSvc());
  setModeSvc()->setHttpHandler([this](WebContext* svr){this->setMode(svr);});
  setDisplayName("Fan Control");
  setModeSvc()->setDisplayName("Set Mode");

/**
 *   Setup Configuration handlers
 */
  setConfiguration()->setHttpHandler([this](WebContext* svr){this->setFanControlConfiguration(svr);});
  setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfiguration()->setHttpHandler([this](WebContext* svr){this->getFanControlConfiguration(svr);});

/**
 *   Setup Timer to cache humidity every 2 seconds. If Humidity exceeds threshold, turn fan ON.
 */
  setHumRefresh(2);
  _timer.setHandler([this]{this->timerCallback();});
}

FanControl::FanControl(const char* type, const char* target) : Control(type, target),_setModeSvc("urn:LeelanauSoftwareCo-com:service:setMode:1","setMode") {
  addService(setModeSvc());
  setModeSvc()->setHttpHandler([this](WebContext* svr){this->setMode(svr);});
  setDisplayName("Fan Control");
  setModeSvc()->setDisplayName("Set Mode");

/**
 *   Setup Configuration handlers
 */
  setConfiguration()->setHttpHandler([this](WebContext* svr){this->setFanControlConfiguration(svr);});
  setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfiguration()->setHttpHandler([this](WebContext* svr){this->getFanControlConfiguration(svr);});

/**
 *   Setup Timer to cache humidity every 2 seconds. If Humidity exceeds threshold, turn fan ON.
 */
  setHumRefresh(2);
  _timer.setHandler([this]{this->timerCallback();});
}

void FanControl::checkThreshold() {
   if( isMode(AUTOMATIC) ) {
    float delta = getHumidity() - (float)getThreshold();
/**
 *    Fan goes ON when humidity exceeds the threshold (delta > 0.5). The extra 0.5% is used to keep the fan from thrashing between ON/OFF 
  *   at the threshold boundary.
 */
      if( delta > 0.5 ) {
         if( fanIsOFF() ) {
            if( loggingLevel(INFO) ) Serial.printf("FanControl::checkThreshold: Humidity %f exceeds Threshold %d, Turning Fan ON\n",getHumidity(),getThreshold());
            setControlState(ON);
         }
      }
 /**
  *   Fan goes OFF if it's ON and humidity is below the threshold (by 0.5%). The extra 0.5% is used to keep the fan from thrashing between ON/OFF 
  *   at the threshold boundary.
  */
      else {
        if( fanIsON() && (delta < -0.5) ) {
          if( loggingLevel(INFO) ) Serial.printf("FanControl::checkThreshold: Humidity %f no longer exceeds Threshold %d, Turning Fan OFF\n",getHumidity(),getThreshold());
          setControlState(OFF);
        }
      }
   }
}

/**
 *  When setting mode to MANUAL, ControlState can optionally be set. When set to AUTOMATIC, any STATE argument is ignored. In all cases, MODE must be the first
 *  argument. The possible combinations for setting Mode are:
 *  1. MODE=AUTOMATIC
 *  2. MODE=MANUAL&STATE=ON
 *  3. MODE=MANUAL&STATE=OFF
 *  
 */
void FanControl::setMode(WebContext* svr) {
   int numArgs = svr->argCount();
   if( numArgs > 0 ) {
     const String& argName = svr->argName(0);
     const String& arg = svr->arg(0);
     if(argName.equalsIgnoreCase("MODE")) {
        if( arg.equalsIgnoreCase("AUTOMATIC")) {
          setMode(AUTOMATIC);
          if( loggingLevel(FINE) ) Serial.printf("FanControl::setMode: Setting MODE to AUTOMATIC\n");
          checkThreshold();
          content(svr);
        }
        else if(arg.equalsIgnoreCase("MANUAL") ) {
          setMode(MANUAL);
          if( numArgs > 1 ) {
            const String& arg2 = svr->arg(1);
            const String& argName2 = svr->argName(1);
            if( loggingLevel(FINE) ) Serial.printf("FanControl::setMode: Setting MODE to MANUAL and %s to %s\n",argName2.c_str(),arg2.c_str());
            if( argName2.equalsIgnoreCase("STATE")) {
              if( arg2.equalsIgnoreCase("ON")) setControlState(ON);
              else setControlState(OFF);
            }
            else {
              if( loggingLevel(FINE) ) Serial.printf("FanControl::setMode: Setting MODE to MANUAL with no additional arguments\n");
            }
          }
          content(svr);
        }
     }
   }
}

void  FanControl::content(WebContext* svr) {
  char buffer[1000];
  int size = sizeof(buffer);
  int pos = 0;
  pos = formatBuffer_P(buffer,size,pos,html_header);
  if( loggingLevel(FINE) ) Serial.printf("FanControl::content: Fan mode is %s and state is %s\n",controlMode(), controlState());
  switch(getControlMode()) {
     case AUTOMATIC:
        pos = formatBuffer_P(buffer,size,pos,auto_on);          
        if( fanIsON() ) pos = formatBuffer_P(buffer,size,pos,fan_on); 
        else pos = formatBuffer_P(buffer,size,pos,fan_off); 
        pos = formatBuffer_P(buffer,size,pos,threshold_setting,getThreshold());       
     break;
     case MANUAL:
        if( fanIsON() ) {
           pos = formatBuffer_P(buffer,size,pos,auto_off);          
           pos = formatBuffer_P(buffer,size,pos,manual_on);          
           pos = formatBuffer_P(buffer,size,pos,fan_on);          
        }
        else {
           pos = formatBuffer_P(buffer,size,pos,auto_off);          
           pos = formatBuffer_P(buffer,size,pos,manual_off);          
           pos = formatBuffer_P(buffer,size,pos,fan_off);          
        }
        pos = formatBuffer_P(buffer,size,pos,threshold_setting,getThreshold());       
     break;
  }
  formatTail(buffer,size,pos); 
  if( loggingLevel(FINE) ) Serial.printf("FanControl::content: Sending %d bytes\n\n",pos);                             
  svr->send(200,"text/html",buffer);
}

void FanControl::setControlState(ControlState flag) {

  if( loggingLevel(FINE) ) Serial.printf("FanControl::setControlState: On entry, Fan state is %s\n",ControlState());

/**
 *  If ControlState is set to ON, send HIGH to the relay
 */
  if(flag == ON) {
    digitalWrite(getPin(),HIGH);
    if( loggingLevel(FINE) ) Serial.printf("FanControl::setControlState: Fan turned ON, set to %d\n", HIGH);
  }
/**
 *  Otherwise send LOW
 */
  else {
    digitalWrite(getPin(),LOW);
    if( loggingLevel(FINE) ) Serial.printf("FanControl::setControlState: Fan turned OFF, set to %d\n", LOW);
  }
  if( loggingLevel(FINE) ) Serial.printf("FanControl::setControlState: Fan state is now %s\n",controlState());
}

void FanControl::configForm(WebContext* svr) {
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
  
  pos = formatBuffer_P(buffer,size,pos,humidity_display,getHumidity());         
  pos = formatBuffer_P(buffer,size,pos,FanControl_config_form,svcPath,getDisplayName(),getThreshold(),pathBuff);

/**
 *  Config Form HTML Tail
 */
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer); 
}

void FanControl::setFanControlConfiguration(WebContext* svr) {
  int numArgs = svr->argCount();
  for( int i=0; i<numArgs; i++) {
     const String& argName = svr->argName(i);
     const String& arg = svr->arg(i);
     if( argName.equalsIgnoreCase("HUMIDITY") ) {
        int t = (int) arg.toInt();
        if( t > 0 ) setThreshold(t);
     }
     else if( argName.equalsIgnoreCase("DISPLAYNAME") ) {if( arg.length() > 0 ) setDisplayName(arg.c_str());}
  }
  display(svr);  
}

void FanControl::getFanControlConfiguration(WebContext* svr) {
  char buffer[1000];
  size_t bufferSize = sizeof(buffer);
  int size = bufferSize;
  snprintf_P(buffer,size,FanControl_config_template,getDisplayName(),getThreshold());
  svr->send(200, "text/xml", buffer);     
}

Thermometer* FanControl::getThermometer() {
   Thermometer*       result          = NULL;
   const ClassType*   thermometerType = Thermometer::classType();
   RootDevice*        root            = rootDevice();
   if(root != NULL ) {
      UPnPDevice* d = root->getDevice(thermometerType);
      if( d != NULL ) result = (Thermometer*)(d->as(thermometerType));
   }
   return result;
}

void FanControl::setup(WebContext* svr) {
  Control::setup(svr);
  pinMode(getPin(),OUTPUT);
  digitalWrite(getPin(),LOW);
  _timer.start();
  if( loggingLevel(FINE) ) Serial.printf("FanControl::setup: Digital pin %d initialized to %d and set to %d\n",getPin(),OUTPUT,LOW);
}

void FanControl::doDevice() {
   Control::doDevice();
   _timer.doDevice();
}

} // End of namespace lsc
