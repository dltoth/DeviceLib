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

const char* MONTHS[12]                   = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

const char  clock_body[]         PROGMEM = "<p align=\"center\"> %s %s </p>";
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
const char  SoftwareClock_config_form[] PROGMEM = "<br><br><form action=\"%s\">"                                                                                 // Service path
            "<div align=\"center\">"
              "<label><b>&nbsp&nbsp&nbsp&nbsp&nbspTime/Date:</b> %s %s </label><br><br>"                                                                        // Time Date
              "<label><b>Last Refresh:</b> %s %s</label><br><br>"                                                                                               // Last refresh Time Date
              "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Refresh</button>"                                            // Refresh handler path
            "</div><br><br><br>"
            "<div align=\"center\">"
              "<label for=\"displayName\">Sensor Name &nbsp &nbsp</label>"
              "<input type=\"text\" placeholder=\"%s\" name=\"displayName\"><br><br>"                                                                           // Device display name
              "<label for=\"tz\">Timezone Offset &nbsp </label>"
              "<input type=\"number\" name=\"tz\" min=\"-12\" max=\"12\" style=\"width:2.1em;font-size:1em\" maxlength=\"3\" value=\"%d\">&nbsp<br><br>"        // Time zone      
              "<label for=\"refresh\">Time Server Refresh (Minutes)</label>"
              "<input type=\"number\" name=\"refresh\" min=\"30\" max=\"1440\" style=\"width:2.1em;font-size:1em\" maxlength=\"3\" value=\"%d\">&nbsp<br><br>"  // UTC refresh
              "<button class=\"fmButton\" type=\"submit\">Submit</button>&nbsp&nbsp"
              "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button>"                                             // Device path
            "</div></form>";
/**
 *  Static RTT initialization
 */
INITIALIZE_STATIC_TYPE(GetDateTime);
INITIALIZE_STATIC_TYPE(SoftwareClock);
INITIALIZE_UPnP_TYPE(GetDateTime,urn:LeelanauSoftware-com:service:getDateTime:1);
INITIALIZE_UPnP_TYPE(SoftwareClock,urn:LeelanauSoftware-com:device:SoftwareClock:1);
NTPTime SoftwareClock::_ntp;

GetDateTime::GetDateTime() : UPnPService("getDateTime") {setDisplayName("Get Date/Time");};
void GetDateTime::handleRequest(WebContext* svr) {
  SoftwareClock* c = (SoftwareClock*)GET_PARENT_AS(SoftwareClock::classType());
  char buffer[256];
  int result = 200;
  if( c != NULL ) {
    int m,d,y,h,min,s;
    c->getDate(m,d,y);
    c->getTime(h,min,s);
    snprintf_P(buffer,256,datetime_template,MONTHS[m-1],d,y,h,min,s);
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
  setConfiguration()->setHttpHandler([this](WebContext* svr){this->setClockConfiguration(svr);});
  setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfiguration()->setHttpHandler([this](WebContext* svr){this->getClockConfiguration(svr);});
}

SoftwareClock::SoftwareClock(const char* target) : Sensor(target) {
  addServices(&_getDateTime);
  setDisplayName("Software Clock");
  setConfiguration()->setHttpHandler([this](WebContext* svr){this->setClockConfiguration(svr);});
  setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
  getConfiguration()->setHttpHandler([this](WebContext* svr){this->getClockConfiguration(svr);});
}

void SoftwareClock::getDate(char* buffer, int len, long t) {
  int m,d,y;
  _ntp.utcDate(now(),m,d,y);
  snprintf(buffer,len,"%s %02d, %d",MONTHS[m-1],d,y);
}

void SoftwareClock::getTime(char* buffer, int len, long t) {
  snprintf(buffer,len,"%02d:%02d:%02d",_ntp.utcHour(t),_ntp.utcMinute(t),_ntp.utcSecond(t));
}


void SoftwareClock::content(char buffer[], int bufferSize) {
  char d[64];
  char t[64];
  getDate(d,64);
  getTime(t,64);
  snprintf_P(buffer,bufferSize,clock_body,t,d);  
}

void SoftwareClock::setClockConfiguration(WebContext* svr) {
  int numArgs = svr->argCount();
  for( int i=0; i<numArgs; i++) {
     const String& argName = svr->argName(i);
     const String& arg = svr->arg(i);
     Serial.printf("SoftwareClock::setConfiguration: Processing argument %s\n",argName.c_str());
     if( argName.equalsIgnoreCase("TZ") ) {
        int val = (int) arg.toInt();
        setTimezone(val);
     }
     else if( argName.equalsIgnoreCase("REFRESH") ) setRefresh((int) arg.toInt());
     else if( argName.equalsIgnoreCase("DISPLAYNAME") ) {if( arg.length() > 0 ) setDisplayName(arg.c_str());}
  }
  display(svr);
}

void SoftwareClock::getClockConfiguration(WebContext* svr) {
  char buffer[1000];
  int size = sizeof(buffer);
  int tz = getTimezone();
  int r  = getRefresh();
  snprintf_P(buffer,size,SoftwareClock_config_template,getDisplayName(),tz,r);
  svr->send(200, "text/xml", buffer);    
}

void SoftwareClock::setup(WebContext* svr) {
  Sensor::setup(svr);
  char pathBuffer[100];
  handlerPath(pathBuffer,100,"refreshUTC");
  svr->on(pathBuffer,[this](WebContext* svr){this->refreshUTC(svr);});
}

void SoftwareClock::refreshUTC(WebContext* svr) {
  _ntp.updateUTC();
  configForm(svr);
}

void SoftwareClock::configForm(WebContext* svr) {
/**
 *    Config Form HTML Start with Title
 */
  char buffer[1500];
  int size = sizeof(buffer);
  int pos = formatHeader(buffer,size,"Set Timezone and UTC Server Refresh");

/**
 *  Current Time/Date into local contentBuffer
 */
  char contentBuff[128];
  int contentSize = sizeof(contentBuff);
  content(contentBuff,contentSize);

/**
 *  Config Form Content
 */
  char pathBuff[100];
  getPath(pathBuff,100);                       // Device path
  char refreshPath[100];
  handlerPath(refreshPath,100,"refreshUTC");   // Refresh handler path
  long currentDateTime = _ntp.now(_tzOffset);
  char d[64];                                  
  char t[64];
  getDate(d,64,currentDateTime);               // Current date
  getTime(t,64,currentDateTime);               // Current time
  long lastSyncTime = _ntp.lastSync(_tzOffset);
  char ls_d[64];
  char ls_t[64];
  getDate(ls_d,64,lastSyncTime);                // Last sync date
  getTime(ls_t,64,lastSyncTime);                // Last sync time
  int tz = getTimezone();                       // Time zone offset
  int r = _ntp.getRefresh();                    // UTC refresh interval
  char svcPath[100];
  setConfiguration()->getPath(svcPath,100);     // Form submit path (service path)
  pos = formatBuffer_P(buffer,size,pos,SoftwareClock_config_form,svcPath,t,d,ls_t,ls_d,refreshPath,getDisplayName(),tz,r,pathBuff);

/**
 *  Config Form HTML Tail
 */
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer); 
}

Timer::Timer( const Timer& t ) {
  _millis   = t._millis;
  _setPoint = t._setPoint;
  _handler  = t._handler;
}

void Timer::setPoint(int& h, int& m, int& s) {
  unsigned long seconds = setPointMillis()/1000;
  h = seconds/3600;
  seconds = seconds % 3600;
  m = seconds/60;
  s = seconds % 60; 
}

void Timer::doDevice() {
  if(_setPoint > 0) {
    if((elapsedTimeMillis() > setPointMillis()) && (_handler!=NULL)) {
      reset();
      _handler();
    }
  }
}

} // End of namespace lsc
