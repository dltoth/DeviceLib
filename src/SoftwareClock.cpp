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

#include "SoftwareClock.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

const char  clock_body[]         PROGMEM = "<p align=\"center\" style=\"font-size:1.35em;\"> %s </p>"
            "<div align=\"center\">"
              "<table>"
              "<tr><td><b>Clock Start:</b></td><td>&ensp;%s</td></tr>"         // Start time
              "<tr><td><b>Running Time:</b></td><td>&ensp;%s</td></tr>"        // Running time
              "<tr><td><b>Last Sync:</b></td><td>&ensp;%s at %s</td></tr>"     // Last NTP Sync  
              "<tr><td><b>Next Sync:</b></td><td>&ensp;%s at %s</td></tr>"     // Next NTP Sync  
              "</table>"
            "</div><br><br>";

const char  root_clock_body[]    PROGMEM = "<p align=\"center\" style=\"font-size:1.1em;\"> %s </p>";
const char  success_template[]   PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><success> Timezone %d Refresh %d</success>";                                       
const char  datetime_template[]  PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><datetime>"
                                            "<date>"
                                               "<month>%s</month>"
                                               "<day>%d</day>"
                                               "<year>%d</year>"
                                            "</date>"
                                            "<time>"
                                                "<hour>%02d</hour>"
                                                "<min>%02d</min>"
                                                "<sec>%02d</sec>"
                                            "</time></datetime>";
const char  SoftwareClock_config_template[]  PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                            "<config>"
                                               "<displayName>%s</displayName>"
                                               "<tz>%d</tz>"
                                               "<refresh>%d</refresh>"
                                            "</config>";
const char  SoftwareClock_config_form[] PROGMEM = "<br><br><form action=\"%s\">"                                                                           // Service path
            "<div align=\"center\">"
              "<p align=\"center\" style=\"font-size:1.35em;\"> %s </p>"                                                                                   // Current time
              "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Sync NTP</button>&ensp;"                                // Sync NTP path
              "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Reset</button><br>"                                     // Reset path
            "</div><br><br><br><br>"
            "<div align=\"center\">"
              "<table>"
              "<tr><td><b><label for=\"displayName\">Control Name</label></b></td>"
                  "<td>&ensp;<input type=\"text\" placeholder=\"%s\" size=\"16\" name=\"displayName\"></td></tr>"                                          // Device display name    
              "<tr><td><b><label for=\"tz\">Timezone</label></b></td>"
                  "<td>&ensp;<input type=\"text\" name=\"tz\" value=\"%s\" pattern=\"^([+-]?[0-9]{1,2}:[0-9]{2})$\" ></td></tr>"                           // Timezone
              "<tr><td><b><label for=\"ntpSync\">NTP Refresh</label></b></td>"
                  "<td>&ensp;<input type=\"number\" name=\"ntpSync\" min=\"15\" maxlength=\"4\" value=\"%d\"></td></tr>"                                   // NTP refresh
              "</table><br><br>"
              "<button class=\"fmButton\" type=\"submit\">Submit</button>&ensp;"
              "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button><br><br>"                                // Device path
            "</div></form>";

/**
 *  Static RTT/UPnP type initialization
 */
INITIALIZE_SERVICE_TYPES(GetDateTime,LeelanauSoftware-com,getDateTime,1.0.0);
INITIALIZE_DEVICE_TYPES(SoftwareClock,LeelanauSoftware-com,SoftwareClock,2.1.1);

GetDateTime::GetDateTime() : UPnPService("getDateTime") {setDisplayName("Get Date/Time");};
void GetDateTime::handleRequest(WebContext* svr) {
  SoftwareClock* c = (SoftwareClock*)GET_PARENT_AS(SoftwareClock::classType());
  char buffer[256];
  int result = 200;
  if( c != NULL ) {
    Instant current = c->now();
    Date date = current.toDate();
    Time time = current.toTime();
    snprintf_P(buffer,256,datetime_template,Instant::MONTHS[date.month-1],date.day,date.year,time.hour,time.min,time.sec);
  }
  else {
    result = 500;
    snprintf_P(buffer,128,error_html,"SoftwareClock");
  }
  svr->send(result, "text/xml", buffer);  
}

SoftwareClock::SoftwareClock() : Sensor("clock") {
  addServices(&_getDateTime);
  setDisplayName("Software Clock");
}

SoftwareClock::SoftwareClock(const char* target) : Sensor(target) {
  addServices(&_getDateTime);
  setDisplayName("Software Clock");
}

int SoftwareClock::formatContent(char buffer[], int size, int pos) {
  char date[64];
  char nst[64];
  char nsd[64];
  char lst[64];
  char lsd[64];
  char start[64];
  char running[64];
  Instant t  = now();
  Instant ns = nextSync();
  Instant ls = lastSync();
  Instant s  = startTime().ntpTime().toTimezone(-4.0);
  t.printDateTime(date,64);
  t.printElapsedTime(s, running, 64);
  s.printDateTime(start,64);
  ns.printDate(nsd,64);
  ns.printTime(nst,64);
  ls.printTime(lst,64);
  ls.printDate(lsd,64);
 
  pos = formatBuffer_P(buffer,size,pos,clock_body,date,start,running,lsd,lst,nsd,nst);  
  return pos;
}

int SoftwareClock::formatRootContent(char buffer[], int size, int pos) {
  char date[64];
  Instant t = now();
  t.printDateTime(date,64);
  pos = formatBuffer_P(buffer,size,pos,root_clock_body,date);  
  return pos;
}

void SoftwareClock::handleSetConfiguration(WebContext* svr) {
  int numArgs = svr->argCount();
  for( int i=0; i<numArgs; i++) {
     const String& argName = svr->argName(i);
     const String& arg = svr->arg(i);
     if( argName.equalsIgnoreCase("TZ") ) {
        double tz = (double)getHours(arg) +(double)getMinutes(arg)/60.;
        setTimezone(tz);
     }
     else if( argName.equalsIgnoreCase("REFRESH") ) setNTPSync((int) arg.toInt());
     else if( argName.equalsIgnoreCase("DISPLAYNAME") ) {if( arg.length() > 0 ) setDisplayName(arg.c_str());}
  }
  display(svr);
}

void SoftwareClock::handleGetConfiguration(WebContext* svr) {
  char buffer[1000];
  int    size = sizeof(buffer);
  double tz   = getTimezone();
  int    r    = getNTPSync();
  snprintf_P(buffer,size,SoftwareClock_config_template,getDisplayName(),tz,r);
  svr->send(200, "text/xml", buffer);    
}

void SoftwareClock::setup(WebContext* svr) {
  Sensor::setup(svr);
  char pathBuffer[100];
  char resetPath[100];
  handlerPath(pathBuffer,100,"refreshNTP");
  handlerPath(resetPath,100,"resetClock");
  svr->on(pathBuffer,[this](WebContext* svr){this->refreshNTP(svr);});
  svr->on(resetPath,[this](WebContext* svr){this->resetClock(svr);});
  updateSysTime();
}

void SoftwareClock::configForm(WebContext* svr) {
/**
 *    Config Form HTML Start with Title
 */
  char buffer[1500];
  int size = sizeof(buffer);
  int pos = formatHeader(buffer,size,"Set Configuration");

/**
 *  Config Form Content
 */
  char current[64];
  now().printDateTime(current,64);
  char pathBuff[100];
  getPath(pathBuff,100);                       // Device path
  char ns_d[64];
  char ns_t[64];
  Instant t = nextSync(); 
  t.printDate(ns_d,64);
  t.printTime(ns_t,64);
  int r =  getNTPSync();                        // NTP refresh interval
  char svcPath[100];
  setConfigurationSvc()->getPath(svcPath,100); // Form submit path (service path)
  char tzBuff[8];
  int h = (int)getTimezone();
  int m = (getTimezone() - h)*60;
  m = ((m<0)?(-m):(m));
  if(h<0) snprintf(tzBuff,8,"-%02d:%02d",-h,m); // in order to force leading zeros in hours place
  else snprintf(tzBuff,8,"+%02d:%02d",h,m);

  char refreshPath[100];
  char resetPath[100];
  handlerPath(refreshPath,100,"refreshNTP");   // Refresh handler path
  handlerPath(resetPath,100,"resetClock");     // Reset handler path
  pos = formatBuffer_P(buffer,size,pos,SoftwareClock_config_form,svcPath,current,refreshPath,resetPath,getDisplayName(),tzBuff,r,pathBuff);

/**
 *  Config Form HTML Tail
 */
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer);
}

/** 
 *  Returns the hours part of time string (+/-)hh:mm as (+/-)int
 *  Result will be >= -14 and <= 14.
 *  Returns 0 if time format is incorrect or hours is out of range.
 */
int SoftwareClock::getHours(const String& s) {
   int result = 0;
   int index = s.indexOf(':');
   String hourStr = s.substring(0,index);
   if(hourStr.startsWith("-")) {
     if( hourStr.charAt(1)  == '0' ) result = -(hourStr.substring(2).toInt());
     else result = -(hourStr.substring(1).toInt());
   }
   else if(hourStr.startsWith("+")) {
     if( hourStr.charAt(1)  == '0' ) result = hourStr.substring(2).toInt();
     else result = hourStr.substring(1).toInt();
   }
   else {
     if( hourStr.charAt(0)  == '0' ) result = hourStr.substring(1).toInt();
     else result = hourStr.substring(0).toInt();
   }

   if( (result > 14 ) || (result < -14) ) result = 0;
   return result;
}

/** 
 *  Returns the minutes part of time string hh:mm as int
 *  Result will be 0, 15, 30, or 45
 *  Returns 0 if time format is incorrect or minutes is out of range.
 */
 int SoftwareClock::getMinutes(const String& s) {
   int result = 0;
   int index = s.indexOf(':');
   String minStr = s.substring(index+1);
   result = minStr.toInt();
   if( (result >= 0) && (result < 8))  result = 0;
   else if( (result >= 8) && (result < 23) ) result = 15;
   else if( (result >= 23) && (result < 38) ) result = 30;
   else result = 45;
   if(s.charAt(0) == '-') result = -result;
   return result; 
}

} // End of namespace lsc
