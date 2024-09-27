/**
 * 
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

#ifndef CUSTOMCLOCK_H
#define CUSTOMCLOCK_H
#include <SystemClock.h>
#include <SensorDevice.h>

class CustomClock : public Sensor {
    public:
    CustomClock() : Sensor("customClock") {setDisplayName("Custom Clock");}
    CustomClock( const char* target ) : Sensor(target) {setDisplayName("Custom Clock");}
    virtual ~CustomClock() {}

    void                     setTimezone(double hours)                {_sysClock.tzOffset(hours);}
    double                   getTimezone()                            {return _sysClock.tzOffset();}

/**
 *   Virtual Functions required for UPnPDevice
 */
    void                     setup(WebContext* svr)                   { Sensor::setup(svr);_sysClock.updateSysTime();}
    void                     doDevice()                               {_sysClock.doDevice();}

/**
 *   Virtual Functions required by Sensor
 */
     int                   formatContent(char buffer[], int bufferSize, int pos);
     int                   formatRootContent(char buffer[], int bufferSize, int pos);

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

    protected:

      SystemClock     _sysClock;  

/**
 *   Copy construction and assignment are not allowed
 */
     DEFINE_EXCLUSIONS(CustomClock);         
};

#endif
