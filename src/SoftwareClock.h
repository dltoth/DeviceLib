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
#include <SystemClock.h>
#include "SensorDevice.h"
#define NTP_SYNC       120     // Synchronize system time with NTP every 2 hours

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
 *  It is designed to fetch NTP time and synchronize to a common system time (sysTime()) on a set interval.
 */
class SoftwareClock : public Sensor {
    public:
    SoftwareClock();
    SoftwareClock( const char* target );
    virtual ~SoftwareClock() {}

    virtual void             setTimezone(double hours)                {_sysClock.tzOffset(hours);}
    virtual void             initialize(const Instant& ref)           {_sysClock.initialize(ref);}
    virtual const Instant&   initializationDate()                     {return _sysClock.initializationDate();}
    virtual double           getTimezone()                            {return _sysClock.tzOffset();}
    virtual void             setNTPSync(unsigned int mins)            {_sysClock.ntpSync(mins);}
    virtual unsigned int     getNTPSync()                             {return _sysClock.ntpSync();}
    virtual Instant          lastSync()                               {return _sysClock.lastSync();}
    virtual Instant          nextSync()                               {return _sysClock.nextSync();}
    virtual Instant          now()                                    {return _sysClock.now();}
    virtual Instant          sysTime()                                {return _sysClock.sysTime();}
    virtual Instant          updateSysTime()                          {return _sysClock.updateSysTime();}
    virtual void             reset()                                  {_sysClock.reset();}
    virtual const Timestamp& startTime()  const                       {return _sysClock.startTime();}

/**
 *   Virtual Functions required for UPnPDevice
 */
    void                     setup(WebContext* svr);
    void                     doDevice()                               {_sysClock.doDevice();}

/**  Form handlers for NTP Refresh and clock reset
 *   
 */
void                       refreshNTP(WebContext* svr)                {updateSysTime();display(svr);}
void                       resetClock(WebContext* svr)                {reset();display(svr);}

/**
 *   Virtual Functions required by Sensor
 */
     int                   formatContent(char buffer[], int bufferSize, int pos);
     int                   formatRootContent(char buffer[], int bufferSize, int pos);

/**
 *    Methods to customize configuration (defined and set on Sensor)
 */
      void                 configForm(WebContext* svr);
      void                 handleSetConfiguration(WebContext* svr);
      void                 handleGetConfiguration(WebContext* svr);
/**
 *    Return hours (or minutes) parf of the String (+/-)HH:MM returned from the config form
 */
      static int           getHours(const String& s);
      static int           getMinutes(const String& s);

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
    
      static const char* MONTHS[12];

    protected:
      GetDateTime     _getDateTime;

      SystemClock     _sysClock;  

/**
 *   Copy construction and assignment are not allowed
 */
     DEFINE_EXCLUSIONS(SoftwareClock);         
};

} // End of namespace lsc

#endif
