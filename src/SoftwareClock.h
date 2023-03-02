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

#ifndef SOFTWARECLOCK_H
#define SOFTWARECLOCK_H
#include <SensorDevice.h>
#include "NTPTime.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

class GetDateTime : public UPnPService {
    public:
    GetDateTime();
    void handleRequest(WebContext* svr);
    
/**
 *   Macros to define the following Runtime Type Info:
 *     private: static const ClassType  _classType;             
 *     public:  static const ClassType* classType();   
 *     public:  virtual void*           as(const ClassType* t);
 *     public:  virtual boolean         isClassType( const ClassType* t);
 */
    DEFINE_RTTI;
    DERIVED_TYPE_CHECK(UPnPService);

/**
 *   Copy construction and destruction are not allowed
 */
     DEFINE_EXCLUSIONS(GetDateTime);         

};

/**
 *  Software Clock is a configurable sensor that provides date and time, synchronized with an NTP server.
 *  It is designed to fetch NTP time and synchronize to a common system time (sysTime()) at
 *  only on a set interval, regardless of the number of SoftwareClocks present in a UPnPDevice.
 */
class SoftwareClock : public Sensor {
    public:
      SoftwareClock();
      SoftwareClock( const char* target );

      void           getTime(char* buffer, int len,long t);
      void           getDate(char* buffer, int len, long t);
      void           getTime(char* buffer, int len)              {getTime(buffer,len,now());}
      void           getDate(char* buffer, int len)              {getDate(buffer,len,now());}
      void           getTime(int& h, int& m, int& s)             {_ntp.getTime(_tzOffset,h,m,s);}
      void           getDate(int& m, int& d, int& y)             {_ntp.getDate(_tzOffset,m,d,y);}
      void           utcTime(long t, int& h, int& m, int& s)     {_ntp.utcTime(t,h,m,s);}
      void           utcDate(long t, int& m, int& d, int& y)     {_ntp.utcDate(t,m,d,y);}
      void           setTimezone( int offset)                    {_tzOffset = offset;}
      int            getTimezone()                               {return _tzOffset;}
      int            getRefresh()                                {return _ntp.getRefresh();}
      void           setRefresh(int secs)                        {_ntp.setRefresh(secs);}
      long           now()                                       {return _ntp.now(_tzOffset);}

/**
 *   Virtual Functions required for UPnPDevice
 */
      void           setup(WebContext* svr);

/**  Form handler for UTC Refresh
 *   
 */
      void           refreshUTC(WebContext* svr);

/**
 *   Virtual Functions required by Sensor
 */
      void           content(char buffer[], int bufferSize);

/**
 *    Methods to customize configuration
 */
      void           configForm(WebContext* svr);
      void           getClockConfiguration(WebContext* svr);
      void           setClockConfiguration(WebContext* svr);

/**
 *   Macros to define the following Runtime and UPnP Type Info:
 *     private: static const ClassType  _classType;             
 *     public:  static const ClassType* classType();   
 *     public:  virtual void*           as(const ClassType* t);
 *     public:  virtual boolean         isClassType( const ClassType* t);
 *     private: static const char*      _upnpType;                                      
 *     public:  static const char*      upnpType()                  
 *     public:  virtual const char*     getType()                   
 *     public:  virtual boolean         isType(const char* t)       
 */
      DEFINE_RTTI;
      DERIVED_TYPE_CHECK(Sensor);
    
    private:
      GetDateTime     _getDateTime;

      int             _tzOffset = DEFAULT_TIMEZONE;

/**
 *    Static NTPTime used by all SoftwareClocks in synchronization with NTP
 *    Note that no synchronization is done since Arduino is not a multi-threaded environment.
 */
      static NTPTime _ntp;
      
/**
 *   Copy construction and destruction are not allowed
 */
     DEFINE_EXCLUSIONS(SoftwareClock);         
};

/** Timer class
 *  Measure elapsed time or trigger a unit of work after some interval. 
 *  Example:
 *  
 *  Timer t = Timer();
 *        t.set(0,0,15);        // Set an interval of 15 seconds
 *        t.setHandler([this]{  // Set up a callback to print current time and reset the timer
 *            SoftwareClock* c = (SoftwareClock*) searchDevice("LeelanauSoftware-com", "SoftwareClock","1");
 *            if( c != NULL ) {
 *               Serial.printf("Date is %s and Time is %s\n",c->getDate().c_str(),c->getTime().c_str());
 *               t.reset();
 *               t.start();
 *           }
 *           else Serial.printf("SoftwareClock NOT found!\n");
 *           });
 *        t.start();
 *
 */
class Timer {
  public:
  Timer() {}
  Timer( const Timer& t );

  void          start()                        {_millis = millis();} 
  bool          started()                      {return _millis != 0;}
  void          reset()                        {_millis = 0;}
  void          clear()                        {reset();_setPoint=0;}
  void          set(int h, int m, int s)       {h=((h<0)?(0):(h));m=((m<0)?(0):(m));s=((s<0)?(0):(s));_setPoint = 1000*s + 60000*m + 3600000*h;}
  void          set(unsigned long millis)      {_setPoint = millis;}
  long          elapsedTimeMillis()            {return((_millis>0)?(millis() - _millis):(0));}
  long          elapsedTimeSeconds()           {return elapsedTimeMillis()/1000;}
  void          setHandler(CallbackFunction h) {if(h != NULL) _handler=h;}
  unsigned long setPointMillis()               {return _setPoint;}
  void          setPoint(int& h, int& m, int& s);
  void          doDevice();
  
  private:
  unsigned long      _millis   = 0;
  unsigned long      _setPoint = 0;
  CallbackFunction   _handler  = ([]{});
     
};

} // End of namespace lsc

#endif
