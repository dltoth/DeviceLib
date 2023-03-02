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

#ifndef NTP_TIME
#define NTP_TIME

#include <Arduino.h>
#include <WiFiUdp.h>

#define IS_LEAP_YEAR(y) ((((((y)%400)==0) || (((y)%100)!=0)) && ((((y)%4)==0)))?(true):(false))

#define DEFAULT_TIMEZONE 0
#define DEFAULT_REFRESH  60

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

/**
 *   NTPTime is a utility class to provide UTC time from an NTP server. UTC is only fetched on a defined interval, otherwise
 *   time is returned relative to the internal ESP system clock (millis()). The following time servers are queried in this
 *   order, depending on success of host address resolution:
 *      time.google.com
 *      time.apple.com
 *      time-a.nist.gov
 *      
 */
class NTPTime {
    public:

      NTPTime() {}

      void         getDate(int tz, int &m, int &d, int &y);
      void         getTime(int tz, int &h, int &m, int &s);
      long         now(int tz);
      long         sysTime() {return now(0);}
/**
 *    It's more efficient to compute month, day, and year together but min/sec/hour are only one or two operations.
 */
      void         utcDate(long t, int &m, int &d, int &y);
      void         utcTime(long t, int& h, int& m, int& s);    
      long         utcMinute(long t)                   {return (t/60) % 60;}
      long         utcSecond(long t)                   {return t % 60;}
      long         utcHour(long t)                     {return (t/3600) % 24;}
      
      long         lastSync(int tz)                      {return(_lastSync + tz*3600UL);}
      void         setRefresh(int secs)                  {_refresh = ((secs<1439)?((secs>29)?(_refresh=secs):(_refresh)):(_refresh));}
      int          getRefresh()                          {return _refresh;}
      
      long         getUTC();                          // Fetch UTC (seconds since Jan 1, 1970) from a time server. Internal cache is not updated
      long         updateUTC();                       // Fetch UTC and update internal cache.


    private:

      int      _refresh   = DEFAULT_REFRESH;
      long     _sysTime   = 0;                          // Unix System time (secs since Jan 1, 1970)
      long     _nextSync  = 0;                          // System time of next synchronization to NTP
      long     _lastSecs  = 0;                          // Timestamp of last request for system time (in milliseconds)
      long     _lastSync  = 0;                          // Unix System time of last UTC server refresh (secs since Jan 1, 1970)
      
      NTPTime(const NTPTime&)= delete;
      NTPTime& operator=(const NTPTime&)= delete;

};

} // End of namespace lsc

#endif
