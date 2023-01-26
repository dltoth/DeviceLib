/**
 * 
 */

#include "TimerControl.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

const char outlet_on[]            PROGMEM = "<br><div align=\"center\">Outlet is ON</div>";
const char outlet_off[]           PROGMEM = "<br><div align=\"center\">Outlet is OFF</div>";
const char next_on[]              PROGMEM = "<br><div align=\"center\">Next ON at %s</div>";

/**
 * Automatic Slider ON
 */
const char auto_on[]     PROGMEM = "<div align=\"center\"><a href=\"./setMode?MODE=MANUAL\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\" checked>"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Automatic</div>";

/**
 * Automatic Slider OFF
 */
const char auto_off[]    PROGMEM = "<div align=\"center\"><a href=\"./setMode?MODE=AUTOMATIC\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\">"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Manual&emsp;&nbsp;</div><br>";

/**
 * Manual Slider ON
 */
const char manual_on[]   PROGMEM = "<div align=\"center\"><a href=\"./setMode?MODE=MANUAL&STATE=OFF\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\" checked>"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Outlet ON</div>";

/**
 * Manual Slider OFF
 */
const char manual_off[]  PROGMEM = "<div align=\"center\">&ensp;<a href=\"./setMode?MODE=MANUAL&STATE=ON\" class=\"toggle\"><input class=\"toggle-checkbox\" type=\"checkbox\">"
                                   "<span class=\"toggle-switch\"></span></a>&emsp;Outlet OFF</div>";


/**
 *    Variable input to form is service action path, display name placeholder, start time, end time, cancel path
 *    Form is in 3 sections, head, time, and buttons. The time section is variable based on the number of intervals
 *    Form head takes service action path and display name as char*
 */
const char Control_config_form_head[]  PROGMEM = "<form action=\"%s\"><div align=\"center\">"
                                        "<label for=\"displayName\">Sensor Name:</label>&emsp;"
                                        "<input type=\"text\" placeholder=\"%s\" name=\"displayName\">&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;<br><br>";
/**                                        
 *    Form time takes start hours, start mins, end hours, end mins as decimal                                       
 */
const char Control_config_form_time[]  PROGMEM = "<br><label for=\"%s\">Start Time:</label>&emsp;"
                                        "<input id=\"%s\" type=\"time\" name=\"%s\" value=\"%02d:%02d\" pattern=\"[0-9]{2}:[0-9]{2}\"/>&emsp;&emsp;"
                                        "<label for=\"%s\">End Time:</label>&emsp;"
                                        "<input id=\"%s\" type=\"time\" name=\"%s\" value=\"%02d:%02d\" pattern=\"[0-9]{2}:[0-9]{2}\"/>";

/** 
 *    Form buttons takes the cancel path as char*
 */
const char TimerControl_config_form_buttons[]  PROGMEM = "<br><br><button class=\"fmButton\" type=\"submit\">Submit</button>&nbsp&nbsp"
                                        "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button>" 
                                        "</div></form>";
                                      
const char TimerControl_config_template_head[]  PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                                          "<config>"
                                                              "<displayName>%s</displayName>";                                   
const char TimerControl_config_template_time[]  PROGMEM =     "<startTime_%d>%02d:%02d</startTime_%d>"
                                                              "<endTime_%d>%02d:%02d</endTime_%d>";
const char TimerControl_config_template_tail[]  PROGMEM = "</config>";

/**
 *  Static RTT initialization
 */
INITIALIZE_STATIC_TYPE(TimerControl);

TimerControl::TimerControl() : Control("urn:LeelanauSoftwareCo-com:device:TimerControl:1","TimerControl"),
                                 _setModeSvc("urn:LeelanauSoftwareCo-com:service:setMode:1","setMode"), _start{0,0,0}, _end{0,0,0} {
  addService(setModeSvc());
  setModeSvc()->setHttpHandler([this](WebContext* svr){this->setMode(svr);});
  setDisplayName("Outlet Control");
  setModeSvc()->setDisplayName("Set Mode");

/**
 *   Setup Configuration handlers
 */
  setConfiguration()->setHttpHandler([this](WebContext* svr){this->setTimerControlConfiguration(svr);});
  setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfiguration()->setHttpHandler([this](WebContext* svr){this->getTimerControlConfiguration(svr);});

/**
 *   Setup Timer to check intervals every INTERVAL_CHECK seconds.
 */
  _timer.set(INTERVAL_CHECK*1000);
  _timer.setHandler([this]{this->timerCallback();});
}

TimerControl::TimerControl(const char* type, const char* target) : Control(type, target),
                                                                     _setModeSvc("urn:LeelanauSoftwareCo-com:service:setMode:1","setMode"), _start{0,0,0}, _end{0,0,0} {
  addService(setModeSvc());
  setModeSvc()->setHttpHandler([this](WebContext* svr){this->setMode(svr);});
  setDisplayName("Outlet Control");
  setModeSvc()->setDisplayName("Set Mode");

/**
 *   Setup Configuration handlers
 */
  setConfiguration()->setHttpHandler([this](WebContext* svr){this->setTimerControlConfiguration(svr);});
  setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfiguration()->setHttpHandler([this](WebContext* svr){this->getTimerControlConfiguration(svr);});
  
/**
 *   Setup Timer to check intervals every INTERVAL_CHECK seconds.
 */
  _timer.set(INTERVAL_CHECK*1000);
  _timer.setHandler([this]{this->timerCallback();});
}

void TimerControl::checkIntervals() {
/**
 *  checkTimer relies on the presence of a SoftwareClock device. This method does nothing if it's not present.
 */
   if( isMode(AUTOMATIC) ) {
      SoftwareClock* c = getSoftwareClock();
      if( c != NULL ) {
/**
 *       Calculate minutes since midnight
 */
         int mins = 0;
         int h, m, s;
         c->getTime(h,m,s);
         mins = h*60 + m;
         
/**
 *       Compare minutes since midnight to stored timer intervals. Outlet should be ON if the current time is within
 *       any of the timer intervals, otherwise is should be OFF. Note that if start == end the interval is not considered.
 */
         boolean outletON = false;
         for( int i=0; (i<MAX_TIMER_INTERVALS) && !outletON; i++ ) {
/**
 *         Time interval does not wrap around midnight
 */
            if( _start[i] < _end[i] ) {if( (mins >= _start[i]) && (mins < _end[i]) ) outletON = true;}
/**
 *          Time interval wrapped around midnight
 */
            else if( _end[i] < _start[i] ) {if( (mins >= _start[i]) || (mins < _end[i]) ) outletON = true;}
         }
/**
 *          If controlState is OFF but timer indicates is should be ON, set controlState to ON. Likewise, if controlState is ON
 *          but timer indicates it should be OFF, then set controlState to OFF.
 */
            if( outletIsOFF() && outletON ) {
                if( loggingLevel(FINE) ) Serial.printf("TimerControl::checkIntervals: Setting Control state to ON at %02d:%02d\n",h,m);
                setControlState(ON);
            }
            else if( outletIsON() && !outletON ) {
                if( loggingLevel(FINE) ) Serial.printf("TimerControl::checkIntervals: Setting Control state to OFF at %02d:%02d\n",h,m);
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
void TimerControl::setMode(WebContext* svr) {
   int numArgs = svr->argCount();
   if( numArgs > 0 ) {
     const String& argName = svr->argName(0);
     const String& arg = svr->arg(0);
     if(argName.equalsIgnoreCase("MODE")) {
        if( arg.equalsIgnoreCase("AUTOMATIC")) {
          setMode(AUTOMATIC);
          if( loggingLevel(FINE) ) Serial.printf("TimerControl::setMode: Setting MODE to AUTOMATIC\n");
          checkIntervals();
          content(svr);
        }
        else if(arg.equalsIgnoreCase("MANUAL") ) {
          setMode(MANUAL);
          if( loggingLevel(FINE) ) Serial.printf("TimerControl::setMode: Setting MODE to MANUAL\n");
          if( numArgs > 1 ) {
            const String& arg2 = svr->arg(1);
            const String& argName2 = svr->argName(1);
            if( argName2.equalsIgnoreCase("STATE")) {
              if( loggingLevel(FINE) ) Serial.printf("TimerControl::setMode: Setting STATE to %s\n",arg2.c_str());
              if( arg2.equalsIgnoreCase("ON")) setControlState(ON);
              else setControlState(OFF);
            }
          }
          content(svr);
        }
     }
   }
}

void  TimerControl::content(WebContext* svr) {  
  char buffer[1000];
  int size = sizeof(buffer);
  int pos = 0;
  pos = formatBuffer_P(buffer,size,pos,html_header);
  if( loggingLevel(FINE) ) Serial.printf("TimerControl::content: Outlet mode is %s and state is %s\n",controlMode(), controlState());
  switch(getControlMode()) {
     case AUTOMATIC:
        pos = formatBuffer_P(buffer,size,pos,auto_on);          
        if( outletIsON() ) pos = formatBuffer_P(buffer,size,pos,outlet_on); 
        else pos = formatBuffer_P(buffer,size,pos,outlet_off); 
        pos = formatBuffer_P(buffer,size,pos,next_on,nextOn());       
     break;
     case MANUAL:
        if( outletIsON() ) {
           pos = formatBuffer_P(buffer,size,pos,auto_off);          
           pos = formatBuffer_P(buffer,size,pos,manual_on);          
           pos = formatBuffer_P(buffer,size,pos,outlet_on);          
        }
        else {
           pos = formatBuffer_P(buffer,size,pos,auto_off);          
           pos = formatBuffer_P(buffer,size,pos,manual_off);          
           pos = formatBuffer_P(buffer,size,pos,outlet_off);          
        }
     break;
  }
  pos = formatTail(buffer,size,pos); 
  if( loggingLevel(FINE) ) Serial.printf("TimerControl::content: Sending %d bytes\n\n",pos);                             
  svr->send(200,"text/html",buffer);
}

const char* TimerControl::nextOn() {
   SoftwareClock* c = getSoftwareClock();
   if( c != NULL ) {
/**
 *    Calculate minutes since midnight
 */
      int mins = 0;
      int h, m, s;
      c->getTime(h,m,s);
      mins = h*60 + m;
/**
 *    Find the next starting interval based on current time, checking only non-trivial intervals
 */
      int index = 0;
      int minsToStart = 1440;      // Minutes in a day = 24*60;
      for( int i = 0; i < MAX_TIMER_INTERVALS; i++ ) {
         int delta = _start[i] - mins;
         if( (delta > 0) && (delta < minsToStart) && (_start[i] != _end[i]) ) {
            index = i;
            minsToStart = delta;
         }
      }
/**
 *    If minsToStart hasn't changed, then current time is past all start times, so next ON is the earliest start time.
 */
      if( minsToStart == 1440 ) {
         for( int i=0; i<MAX_TIMER_INTERVALS; i++ ) {
            if( (_start[i] < minsToStart) && (_start[i] != _end[i]) ) {
                index = i;
                minsToStart = _start[i];
            }
         }
      }
      
      int sh = _start[index]/60;
      int sm = _start[index]%60;
      snprintf(_nextON,8,"%02d:%02d",sh,sm);
   }
   else snprintf(_nextON,8,"00:00");
   return _nextON; 
}

void TimerControl::setControlState(ControlState flag) {
  
/**
 *  If ControlState is set to ON, send HIGH to the relay
 */
  if(flag == ON) {
    digitalWrite(getPin(),HIGH);
    if( loggingLevel(FINE) ) Serial.printf("TimerControl::setControlState: Outlet turned ON, set to %d\n", HIGH);
  }
/**
 *  Otherwise send LOW
 */
  else {
    digitalWrite(getPin(),LOW);
    if( loggingLevel(FINE) ) Serial.printf("TimerControl::setControlState: Outlet turned OFF, set to %d\n", LOW);
  }
}

void TimerControl::configForm(WebContext* svr) {

/**
 *  Array dimension of 1700 is barely enough 
 */
  char buffer[1700];
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

  pos = formatBuffer_P(buffer,size,pos,Control_config_form_head,svcPath,getDisplayName());
  
  for(int i=0; i<MAX_TIMER_INTERVALS; i++ ) {
     int startHours = _start[i]/60;
     int startMins  = _start[i]%60;
     int endHours   = _end[i]/60;
     int endMins    = _end[i]%60;
     char startName[32];
     char endName[32];
     snprintf(startName,32,"START_TIME_%d",i);
     snprintf(endName,32,"END_TIME_%d",i);
     pos = formatBuffer_P(buffer,size,pos,Control_config_form_time,startName,startName,startName,startHours,startMins,endName,endName,endName,endHours,endMins);
  }

  pos = formatBuffer_P(buffer,size,pos,TimerControl_config_form_buttons,pathBuff);

/**
 *  Config Form HTML Tail
 */
  pos = formatTail(buffer,size,pos);
  if( loggingLevel(FINE) ) Serial.printf("TimerControl::configForm: Sending %d bytes\n",pos);                             
  svr->send(200,"text/html",buffer); 
}

void TimerControl::setTimerControlConfiguration(WebContext* svr) {
  
  int numArgs = svr->argCount();
  for( int i=0; i<numArgs; i++) {
     const String& argName = svr->argName(i);
     const String& arg = svr->arg(i);
 
     if( argName.startsWith("START_TIME_") ) {
        int seqNum = argSequenceNumber(argName);
        if( seqNum >= 0 ) {
           int h = getHours(arg);
           int m = getMinutes(arg);
           if( (h >= 0) && (m >= 0)  ) {
               _start[seqNum] = 60*h + m;
           }
           else {
              if( loggingLevel(WARNING) ) Serial.printf("TimerControl::setTimerControlConfiguration: Error processing number format in arg %s for argument %s\n", arg.c_str(),argName.c_str());                  
           }
        }
        else {
            if( loggingLevel(WARNING) ) Serial.printf("TimerControl::setTimerControlConfiguration: Error processing sequence number for argument %s\n",argName.c_str());
        }
     }
     else if( argName.startsWith("END_TIME_") ) {
        int seqNum = argSequenceNumber(argName);
        if( seqNum >= 0 ) {
           int h = getHours(arg);
           int m = getMinutes(arg);
           if( (h >= 0) && (m >= 0) ) {
               _end[seqNum] = 60*h + m;
           }
           else {
              if( loggingLevel(WARNING) ) Serial.printf("TimerControl::setTimerControlConfiguration: Error processing number format in arg %s for argument %s\n", arg.c_str(),argName.c_str());                  
           }
        }
        else {
            if( loggingLevel(WARNING) ) Serial.printf("TimerControl::setTimerControlConfiguration: Error processing sequence number for argument %s\n",argName.c_str());
        }
     }
     else if( argName.equalsIgnoreCase("DISPLAYNAME") ) {if( arg.length() > 0 ) setDisplayName(arg.c_str());}
  }  
  display(svr);  
}

int TimerControl::argSequenceNumber(const String& argName) {
   int result = -1;
/**
 *  argName should be START_TIME_d where d is a digit less than MAX_TIMER_INTERVALS
 */
   if( argName.length() == 12 ) {
      if(isDigit(argName.charAt(11)) ) result = argName.substring(11,12).toInt();
   }
/**
 *  argName should be END_TIME_d where d is a digit less than MAX_TIMER_INTERVALS
 */
   else if( argName.length() == 10 ) {
      if(isDigit(argName.charAt(9)) ) result = argName.substring(9,10).toInt();   
   }
   
   if( (result < 0) || (result > MAX_TIMER_INTERVALS) ) result = -1;
   return result;
}
/**
 */
void TimerControl::getTimerControlConfiguration(WebContext* svr) {
  char buffer[600];
  size_t bufferSize = sizeof(buffer);
  int size = bufferSize;
  int pos = 0;
  pos = formatBuffer_P(buffer,size,pos,TimerControl_config_template_head,getDisplayName());
  for( int i=0; i<MAX_TIMER_INTERVALS; i++ ) {
     pos = formatBuffer_P(buffer,size,pos,TimerControl_config_template_time,i,_start[i]/60,_start[i]%60,i,i,_end[i]/60,_end[i]%60,i);
  }
  pos = formatBuffer_P(buffer,size,pos,TimerControl_config_template_tail);
  if( loggingLevel(FINE) ) Serial.printf("TimerControl::getTimerControlConfiguration: Sending %d bytes\n",pos);                             
  svr->send(200, "text/xml", buffer);     
}

SoftwareClock* TimerControl::getSoftwareClock() {
   SoftwareClock*     result            = NULL;
   const ClassType*   softwareClockType = SoftwareClock::classType();
   RootDevice*        root              = rootDevice();
   if(root != NULL ) {
      UPnPDevice* d = root->getDevice(softwareClockType);
      if( d != NULL ) result = (SoftwareClock*)(d->as(softwareClockType));
   }
   return result;
}

void TimerControl::setup(WebContext* svr) {
  Control::setup(svr);
  _timer.start();
  pinMode(getPin(),OUTPUT);
  digitalWrite(getPin(),LOW);
  if( loggingLevel(FINE) ) Serial.printf("TimerControl::setup: Digital pin %d initialized to %d and set to %d\n",getPin(),OUTPUT,LOW);
}

/** 
 *  Returns the hours part of time string hh:mm as int
 *  Result will be >=0 and <= 23.
 *  Returns -1 if time format is incorrect or hours is out of range.
 */
int TimerControl::getHours(const String& s) {
   int result = -1;
   if( (s.indexOf(':') == 2) && (s.length() == 5) && isDigit(s.charAt(0)) && isDigit(s.charAt(1)) ) {
      result = s.substring(0,2).toInt();
      if( result > 23 ) result = -1;
   }
   return result;
}

/** 
 *  Returns the minutes part of time string hh:mm as int
 *  Result will be >=0 and <= 59.
 *  Returns -1 if time format is incorrect or minutes is out of range.
 */
 int TimerControl::getMinutes(const String& s) {
   int result = -1;
   if( (s.indexOf(':') == 2) && (s.length() == 5) && isDigit(s.charAt(3)) && isDigit(s.charAt(4)) ) {
      result = s.substring(3).toInt();
      if( result > 59 ) result = -1;
   }
   return result;  
}

void TimerControl::doDevice() {
   Control::doDevice();
   _timer.doDevice();
}
} // End of namespace lsc
