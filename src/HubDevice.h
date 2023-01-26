
/**
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
      HubDevice( const char* type, const char* target );

      void          displayRoot(WebContext* svr);

/**
 *   Logging
 */
     void           logging(LoggingLevel level)                 {_logging = level;}
     LoggingLevel   logging()                                   {return _logging;}
     boolean        loggingLevel(LoggingLevel level)            {return(logging() >= level);}

/**
 *   Macros to define the following Runtime Type Info:
 *     private: static const ClassType  _classType;             
 *     public:  static const ClassType* classType();   
 *     public:  virtual void*           as(const ClassType* t);
 *     public:  virtual boolean         isClassType( const ClassType* t);
 */
      DEFINE_RTTI;
      DERIVED_TYPE_CHECK(ExtendedDevice);
      
      private:

      LoggingLevel         _logging = NONE;

      HubDevice(const HubDevice&)= delete;
      HubDevice& operator=(const HubDevice&)= delete;
      
};

} // End of namespace lsc

#endif
