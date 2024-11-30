/**
 * 
 *  UPnPDevice Library
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

#ifndef CLOCK_DEVICE_H
#define CLOCK_DEVICE_H

#include <SystemClock.h>
#include <DeviceLib.h>

/**
 *   Custom Sensor implementing a SystemClock
**/
class ClockDevice : public Sensor {

    public:
      ClockDevice() : Sensor("clock")                   {setDisplayName("Clock");}
      ClockDevice( const char* target) : Sensor(target) {setDisplayName("Clock");}

      Instant       now()                  {return _clock.now();}
      void          setTimezone(double tz) {_clock.tzOffset(tz);}
      double        getTimezone()          {return _clock.tzOffset();}
      
/**
 *   Virtual Functions required for UPnPDevice
 */
      void           setup(WebContext* svr);

/**
 *   Virtual Functions required by UPnPDevice
 */
      int           formatContent(char buffer[], int size, int pos);
      int           formatRootContent(char buffer[], int size, int pos);

/**
 *   Macros to define the following Runtime Type Info:
 */
      DEFINE_RTTI;
      DERIVED_TYPE_CHECK(Sensor);
    
    protected:

      SystemClock       _clock;

/**
 *   Copy construction and destruction are not allowed
 */
      DEFINE_EXCLUSIONS(ClockDevice);         

};

#endif