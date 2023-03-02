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

#ifndef EXTENDED_DEVICE_H
#define EXTENDED_DEVICE_H

#include <WiFiPortal.h>
#include <WiFiUdp.h>
#include <Control.h>
#include <SensorDevice.h>
#include <Configuration.h>

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {
  
/** ExtendedDevice is a Configurable RootDevice that provides SSDP Search capability for UPnPDevices on the
 *  same local network. Default configuration for ExtendedDevice allows for getting and setting display name, and resetting 
 *  the Access Point. This behavior can be overridden via the setHttpHandler method of the SetConfiguration() 
 *  UPnPService. AccessPoint reset depends on use of the WiFiPortal library.
 *  
 */
class ExtendedDevice : public RootDevice {
  
    public:
      ExtendedDevice();
      ExtendedDevice( const char* target );



/** ExtendedDevice with complex configutation should implement methods for the following
 *  
 *    configForm(WebContext* svr)                  := Presents an HTML form for configuration input. Form submission
 *                                                    ultimately calls setConfiguration() on the ExtendedDevice Object. Default 
 *                                                    action only supplies a form for setting diaplay name.  
 *                                                    Note: the form "action" attribute MUST be "setConfiguration".
 *                                                    This should set on the constructor with:
 *                                                       setConfiguration()->setFormHandler([this](WebContext* svr){this->configForm(svr);});
 *                                                    Note that ExtendedDevice adds to default configuration to allow re-homing networp AP
 *    setExtendedDeviceconfiguration(WebContext* svr) := Request handler for form submission; called from the UPnPService
 *                                                    SetConfiguration::handleRequest(). Default action only sets display name but could be
 *                                                    customized by setting an HTTPHandler on construction with:
 *                                                       setConfiguration()->setHttpHandler([this](WebContext* svr){this->setExtendedDeviceConfiguration(svr);});
 *    getExtendedDeviceConfiguration(WebContext* svr) := Request handler that replies with an XML document describing configuration;
 *                                                    called from the UPnPService GetConfiguration::handleRequest().
 *                                                    here is no formal schema for this document but must start with XML header:
 *                                                    <?xml version=\"1.0\" encoding=\"UTF-8\"?>
 *                                                    And must include: 
 *                                                    <config><displayName>display name</displayName>...</config>                      
 *                                                    Default action only supplies the displayName attribute but could be customized by setting an HTTPHandler 
 *                                                    on construction with:
 *                                                       getConfiguration()->setHttpHandler([this](WebContext* svr){this->getExtendedDeviceConfiguration(svr);});
 *    
 *    For a detailed example of how this is done, see SoftwareClock.
 */

      void              display(WebContext* svr);
      void              displayRoot(WebContext* svr);
      void              setup(WebContext* svr);
      void              configForm(WebContext* svr);
      void              resetAP(WebContext* svr);
      void              clearAP(WebContext* svr);
      GetConfiguration* getConfiguration() {return &_getConfiguration;}
      SetConfiguration* setConfiguration() {return &_setConfiguration;}

/**
 *    Use UPnP SSDP protocol to search for nearby RootDevices enabled with SSDP.
 *    Responds with a Web Page of RootDevice buttons
 */
      virtual void    nearbyDevices(WebContext* svr);

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
      DERIVED_TYPE_CHECK(RootDevice);

/**
 *   Copy construction and destruction are not allowed
 */
      DEFINE_EXCLUSIONS(ExtendedDevice);         

      private:
      GetConfiguration     _getConfiguration;
      SetConfiguration     _setConfiguration;

};

} // End of namespace lsc



#endif
