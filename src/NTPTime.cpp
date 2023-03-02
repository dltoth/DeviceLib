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

#include "NTPTime.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

const int   daysPerMonth[12]             = {31,28,31,30,31,30,31,31,30,31,30,31};
#define     MONTH_DAYS(m,y) ((m==1)?((IS_LEAP_YEAR(y))?(daysPerMonth[m]+1):(daysPerMonth[m])):((daysPerMonth[m])))
#define     NTP_PACKET_SIZE 48

void NTPTime::getDate(int tz, int &m, int &d, int& y) {
  utcDate(now(tz),m,d,y);
}

void NTPTime::getTime(int tz, int &h, int &m, int &s ) {
  long t = now(tz);
  h = utcHour(t);
  m = utcMinute(t);
  s = utcSecond(t);
}

void NTPTime::utcTime(long t, int &h, int &m, int &s) {
  h = utcHour(t);
  m = utcMinute(t);
  s = utcSecond(t);
}
void NTPTime::utcDate(long t, int &m, int &d, int &y) {

/**
 *   86400 secs per day
 *   Compute days since Jan 1, 1970
 */
  int days = t/86400;
  int year = 1970;
  
  while( days >= 0 ) {
    days -= IS_LEAP_YEAR(year) ? 366 : 365;
/**
 *   Increment must be OUTSIDE of macro, otherwise macro expansion will increment 3 times.
 */
    year++;
  }

/**
 *  The loop took us one past where we need to be so we'll have to unwind the last operation
 */
  y = year - 1;
  days += IS_LEAP_YEAR(y) ? 366 : 365;

/**
 *   days is now the day in the year, starting with 0
 */
  int month = 0;
  for( month=0; (month<12) && (days>MONTH_DAYS(month,y)); month++ ) {
    days -= MONTH_DAYS(month,y);
  }
  d = days + 1;
  m = month + 1;
}

long NTPTime::getUTC() {

/** Set up the UDP channel
 *  
 */ 
  WiFiUDP udpChannel;
  int status = udpChannel.begin(0);
  if( status != 1 ) Serial.printf("Error initializing UDP channel (on udpChannel.begin)\n");
  
  long result = 0;
  byte packetBuffer[NTP_PACKET_SIZE];     //buffer to hold incoming & outgoing packets  
  while (udpChannel.parsePacket() > 0) ;  // discard any previously received packets
    
  IPAddress timeServer;
  int err = WiFi.hostByName("time.google.com", timeServer);
  if( err != 1 ) {
    Serial.printf("NTPTime::getUTC: Error resolving time.google.com: %d\n", err);
    err = WiFi.hostByName("time.apple.com", timeServer);
    if( err != 1 ) {
       Serial.printf("NTPTime::getUTC: Error resolving time.apple.com: %d using time-a.nist.gov: 129.6.15.28\n", err);
       timeServer = IPAddress(129,6,15,28);
    }
  }

/**
 *    Send the NTP Request Packet
 *    Initialize values needed to form NTP request
 */
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

/** All NTP fields have been given values, now send a packet on port 123 requesting a timestamp  
 */
  udpChannel.beginPacket(timeServer, 123);
  udpChannel.write(packetBuffer, NTP_PACKET_SIZE);
  status = udpChannel.endPacket();
  if( status != 1 ) Serial.printf("Error writing UDP packet to channel\n");

/**
 *   Read NTP Response
 */
  uint32_t beginWait = millis();
  while (millis() - beginWait < 2000) {
    int size = udpChannel.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      udpChannel.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      // Convert result to time since 1970
      result = secsSince1900 - 2208988800L;
    }
  }

/** Tear down the UDP channel.
 *  
 */
  udpChannel.stop();  
  return result;
}

long NTPTime::updateUTC() {
  long t = getUTC();
  if (t != 0) {
    _sysTime = t;
    _nextSync = _sysTime + _refresh*60;
    _lastSync = _sysTime;
  }   
  return _sysTime;
}

long NTPTime::now( int tzOffset ) {
  
  long currentSecs   = millis()/1000;
  long ellapsedSecs  = currentSecs - _lastSecs;

  if( ellapsedSecs > 0 ) {
     _lastSecs = currentSecs;
     _sysTime += ellapsedSecs;
  }
  
  if( _sysTime > _nextSync ) _sysTime = updateUTC();
  return _sysTime + tzOffset*3600UL;
}

} // End of namespace lsc
