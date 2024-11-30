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

#ifndef CLOCK_WITH_CONFIG_H
#define CLOCK_WITH_CONFIG_H

#include "ClockDevice.h"

/**
 *   Custom Sensor implementing a SystemClock with custom configuration 
**/
class ClockWithConfig : public ClockDevice {

    public:
      ClockWithConfig() : ClockDevice("clock")                   {setDisplayName("Clock");}
      ClockWithConfig( const char* target) : ClockDevice(target) {setDisplayName("Clock");}
     
/**
 *   Virtual methods providing configuration support defined by Sensor
 */
   void           configForm(WebContext* svr);                   // Display device configuration form
   void           handleSetConfiguration(WebContext* svr);       // Form handler for setting configuration
   void           handleGetConfiguration(WebContext* svr);       // Request handler to return configuration as XML

/**
 *   Macros to define the following Runtime Type Info:
 */
      DEFINE_RTTI;
      DERIVED_TYPE_CHECK(ClockDevice);
/**
 *   Copy construction and destruction are not allowed
 */
      DEFINE_EXCLUSIONS(ClockWithConfig);         

};

#endif