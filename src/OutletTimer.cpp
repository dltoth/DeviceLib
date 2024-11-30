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

#include "OutletTimer.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

const char next_on[]      PROGMEM = "<div align=\"center\">ON at %s</div>";
const char next_off[]     PROGMEM = "<div align=\"center\">OFF at %s</div>";

/**
 *    Variable input to form is service action url, display name placeholder, start time, end time, cancel url
 *    Form is in 3 sections, head, time, and buttons. The time section is variable based on the number of intervals
 *    Form head takes service action url and display name as char*
 */
const char timer_config_form_head[]  PROGMEM = "<form action=\"%s\"><div align=\"center\">"
                                        "<label for=\"displayName\">Sensor Name:</label>&emsp;"
                                        "<input type=\"text\" placeholder=\"%s\" name=\"displayName\">&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;<br><br>";
/**                                        
 *    Form time takes start hours, start mins, end hours, end mins as 2 digit integers                                       
 */
const char timer_config_form_time[]  PROGMEM = "<br><label for=\"%s\">Start Time:</label>&emsp;"
                                        "<input id=\"%s\" type=\"time\" name=\"%s\" value=\"%02d:%02d\" pattern=\"[0-9]{2}:[0-9]{2}\"/>&emsp;&emsp;"
                                        "<label for=\"%s\">End Time:</label>&emsp;"
                                        "<input id=\"%s\" type=\"time\" name=\"%s\" value=\"%02d:%02d\" pattern=\"[0-9]{2}:[0-9]{2}\"/>";

/** 
 *    Form buttons takes the cancel path as char*
 */
const char timer_config_form_buttons[]  PROGMEM = "<br><br><div align=\"center\">Note: Intervals may wrap-around midnight but should not overlap</div><br>"
                                       "<br><button class=\"fmButton\" type=\"submit\">Submit</button>&nbsp&nbsp"
                                        "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button>" 
                                        "</div></form>";
                                        
                                      
const char timer_config_template_head[]  PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                                          "<config>"
                                                              "<displayName>%s</displayName>";                                   
const char timer_config_template_time[]  PROGMEM =     "<startTime_%d>%02d:%02d</startTime_%d>"
                                                              "<endTime_%d>%02d:%02d</endTime_%d>";
const char timer_config_template_tail[]  PROGMEM = "</config>";
                                      
/**
 *  Static RTT initialization
 */
INITIALIZE_DEVICE_TYPES(OutletTimer,LeelanauSoftware-com,OutletTimer,1.0.0);

OutletTimer::OutletTimer() : SensorControlledRelay("outletTimer") {
  setDisplayName("Outlet Timer");
}

OutletTimer::OutletTimer(const char* target) : SensorControlledRelay(target) {
  setDisplayName("Outlet Timer");
}

int  OutletTimer::formatContent(char buffer[], int size, int pos) {  
  pos = SensorControlledRelay::formatContent(buffer,size,pos);
  if( isAUTOMATIC() ) {
    if( isON() ) pos = formatBuffer_P(buffer,size,pos,next_off,nextOFF()); 
    else pos = formatBuffer_P(buffer,size,pos,next_on,nextON());
  } 
  return pos;         
}

/**
*   Look at the current time (minutes since midnight) and compare it to every interval. The cycle time is the next
*   ON or next OFF, which ever comes first. If current time is within an interval then cycle time is the endpoint
*   of that interval, otherwise it's the smallest start time greater than current time (wrapping around midnight).
*   If mode is AUTOMATIC and state is ON, then cycle time is the next OFF. Likewise, if mode is AUTOMATIC and state
*   is OFF, then cycle time is next ON.
*/
void OutletTimer::nextCycle() {
   SoftwareClock* c   = getSoftwareClock();
   if( c != NULL ) {
/**
 *    Calculate minutes since midnight
 */
      Time t = c->now().toTime();
      int currentMins = t.hour*60 + t.min;
      int next_ON     = 1440;       // 1440 mins in a day (60*24)
      int next_OFF    = 1440;
      int minStart    = 1440;
      int minEnd      = 1440;
/**
 *    Check all intervals looking for the smallest start and end that are greater than
 *    current time. While we're at it, also find smallest start and smallest end.
 *    If start[i] == end[i] the interval is trivial and not checked, if end[i] < start[i]
 *    the interval wraps around midnight.
 */
      for( int i = 0; i < MAX_TIMER_INTERVALS; i++ ) {
/**
*        Interval does NOT wrap around midnight
*/
         if( _start[i] < _end[i] ) { 
           if( currentMins < _start[i] ) {      
              if( next_ON > _start[i] ) next_ON  = _start[i];
              if( next_OFF > _end[i]  ) next_OFF = _end[i];
           }
           else if( (currentMins < _end[i]) && (next_OFF > _end[i]) ) next_OFF = _end[i];
/**
*          Update minimum start and end
*/
           if( _start[i] < minStart ) minStart = _start[i];
           if( _end[i]   < minEnd   ) minEnd   = _end[i];
         }
/**
*        Interval DOES wrap around midnight
*/
         else if( _end[i] < _start[i] ) {   
           if( (currentMins >= _start[i]) && (next_OFF > _end[i]) )       next_OFF = _end[i];       // Inside interval before midnight  
           else if( (currentMins < _end[i]) && (next_OFF > _end[i]) )     next_OFF = _end[i];       // Inside interval after midnight
           else if( (currentMins < _start[i]) && (next_ON > _start[i]) )  next_ON  = _start[i];     // Outside interval
/**
*          Update minimum start and end
*/
           if( _start[i] < minStart ) minStart = _start[i];
           if( _end[i]   < minEnd   ) minEnd   = _end[i];
         }
         if(loggingLevel(FINEST)) printInterval(currentMins,_start[i],_end[i],next_ON,next_OFF);
      }
      if(loggingLevel(FINEST)) Serial.printf("OutletTimer::nextCycle: minStart is %02d:%02d and minEnd is %02d:%02d\n",
                                             minStart/60,minStart%60,minEnd/60,minEnd%60);
/**
*     If current falls outside of all intervals and is not ahead of
*     a wrap-around interval, next_ON and possibly next_OFF were not set.
*     Use minimums.
*/      
      if( next_ON == 1440 ) next_ON = minStart;
      if( next_OFF == 1440 ) next_OFF = minEnd;            
      snprintf(_nextON,8,"%02d:%02d",next_ON/60,next_ON%60);
      snprintf(_nextOFF,8,"%02d:%02d",next_OFF/60,next_OFF%60);
   }
/**
*  No SoftwareClock present, nextON and nextOFF are trivially 00:00
*/
   else {
      snprintf(_nextON,8,"00:00");
      snprintf(_nextOFF,8,"00:00");
   }

  if(loggingLevel(FINE)) Serial.printf("OutletTimer::nextCycle:   next ON is %s and nextOFF is %s\n",_nextON,_nextOFF);
}

void OutletTimer::printInterval(int currentMins, int start, int end, int nON, int nOFF) {
   Serial.printf("OutletTimer::printInterval: current is %02d:%02d, start is %02d:%02d, end is %02d:%02d, nextON is %02d:%02d, nextOFF is %02d:%02d\n",
           currentMins/60,currentMins%60,start/60,start%60,end/60,end%60,nON/60,nON%60,nOFF/60,nOFF%60);
}

ControlState OutletTimer::sensorState() {
  ControlState result = OFF;
  SoftwareClock* c = getSoftwareClock();
  if( c != NULL ) {
  /**
  *   Calculate minutes since midnight
  */
    Time t = c->now().toTime();
    int currentMins = t.hour*60 + t.min;
    
  /**
  *   Compare minutes since midnight to stored timer intervals. Outlet should be ON if the current time is within
  *   any of the timer intervals, otherwise is should be OFF. Intervals that wrap around midnight have end < start.
  *   The search stops once the result is ON. Note that if start == end the interval is trivial and not considered.
  */
    for( int i=0; (i<MAX_TIMER_INTERVALS) && (result == OFF); i++ ) {
  /**
  *   Time interval does not wrap around midnight
  */
      if( _start[i] < _end[i] ) {
        if( (currentMins >= _start[i]) && (currentMins < _end[i]) ) {
          result = ON;
        }
      }
  /**
  *   Time interval wrapped around midnight
  */
      else if( _end[i] < _start[i] ) {if( (currentMins >= _start[i]) || (currentMins < _end[i]) ) result = ON;}
    }
  }
  return result;
}

void OutletTimer::configForm(WebContext* svr) {
/**
 *  Array dimension of 1700 is barely enough 
 */
  char buffer[1700];
  int size = sizeof(buffer);
  
/**
 *    Config Form HTML Start with Title
 */
  int pos = formatHeader(buffer,size,"Timer Configuration");

/**
 *  Config Form Content
 */ 
  char pathBuff[100];
  getPath(pathBuff,100);
  char svcPath[100];
  setConfigurationSvc()->getPath(svcPath,100);
  pos = formatBuffer_P(buffer,size,pos,timer_config_form_head,svcPath,getDisplayName());
  
  for(int i=0; i<MAX_TIMER_INTERVALS; i++ ) {
     int startHours = _start[i]/60;
     int startMins  = _start[i]%60;
     int endHours   = _end[i]/60;
     int endMins    = _end[i]%60;
     char startName[32];
     char endName[32];
     snprintf(startName,32,"START_TIME_%d",i);
     snprintf(endName,32,"END_TIME_%d",i);
     pos = formatBuffer_P(buffer,size,pos,timer_config_form_time,startName,startName,startName,startHours,startMins,endName,endName,endName,endHours,endMins);
  }

  pos = formatBuffer_P(buffer,size,pos,timer_config_form_buttons,pathBuff);

/**
 *  Config Form HTML Tail
 */
  pos = formatTail(buffer,size,pos);
  if( loggingLevel(FINE) ) Serial.printf("OutletTimer::configForm: Sending %d bytes\n",pos);                             
  svr->send(200,"text/html",buffer); 
}

void OutletTimer::handleSetConfiguration(WebContext* svr) {
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

/** 
 *  Returns the hours part of time string hh:mm as int
 *  Result will be >=0 and <= 23.
 *  Returns -1 if time format is incorrect or hours is out of range.
 */
int OutletTimer::getHours(const String& s) {
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
 int OutletTimer::getMinutes(const String& s) {
   int result = -1;
   if( (s.indexOf(':') == 2) && (s.length() == 5) && isDigit(s.charAt(3)) && isDigit(s.charAt(4)) ) {
      result = s.substring(3).toInt();
      if( result > 59 ) result = -1;
   }
   return result;  
}

int OutletTimer::argSequenceNumber(const String& argName) {
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

void OutletTimer::handleGetConfiguration(WebContext* svr) {
  char buffer[1000];
  size_t bufferSize = sizeof(buffer);
  int size = bufferSize;
  int pos = 0;
  pos = formatBuffer_P(buffer,size,pos,timer_config_template_head,getDisplayName());
  for( int i=0; i<MAX_TIMER_INTERVALS; i++ ) {
     pos = formatBuffer_P(buffer,size,pos,timer_config_template_time,i,_start[i]/60,_start[i]%60,i,i,_end[i]/60,_end[i]%60,i);
  }
  pos = formatBuffer_P(buffer,size,pos,timer_config_template_tail);
  if( loggingLevel(FINE) ) Serial.printf("OutletTimer::getOutletTimerConfiguration: Sending %d bytes\n",pos);                             
  svr->send(200, "text/xml", buffer);     
}

} // End of namespace lsc