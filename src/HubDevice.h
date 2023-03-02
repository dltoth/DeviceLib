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

#ifndef HUBDEVICE_H
#define HUBDEVICE_H

#include <ssdp.h>
#include "ExtendedDevice.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {
  
/** HubDevice is a turnkey UPnPDevice that keeps track of other RootDevices on the local network.
 *
 *  The displayRoot() method, set on '/', will display all RootDevices as HTML buttons.
 *  Configuration for HubDevice is that of ExtendedDevice
 *  
 */
class HubDevice : public ExtendedDevice {
  
    public:
      HubDevice();
      HubDevice( const char* target );

      void          displayRoot(WebContext* svr);

/**
 *   Logging
 */
     void           logging(LoggingLevel level)                 {_logging = level;}
     LoggingLevel   logging()                                   {return _logging;}
     boolean        loggingLevel(LoggingLevel level)            {return(logging() >= level);}

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
      DERIVED_TYPE_CHECK(ExtendedDevice);
      
      private:

      LoggingLevel         _logging = NONE;

/**
 *   Copy construction and destruction are not allowed
 */
     DEFINE_EXCLUSIONS(HubDevice);         
      
};

} // End of namespace lsc

#endif
