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

#ifndef CONTROL_H
#define CONTROL_H

#include <UPnPLib.h>
#include "ConfigurationServices.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

/** A Control is a Configurable Device that provides its Control UI thru an iFrame. 
 *  Configuration is provided by SetConfiguration and GetConfiguration UPnPServices
 *  Implementation Notes:
 *     1. formatContent should provide content for device display from the device target, and can be more complex than content displayed from the RootDevice
 *     2. formatIFrameContent should provide content for device display from the root target, can be simpler than display content from the display target
 *     3. formatRootContent inserts an iFrame into the RootDevice display buffer, target of the iFrame ultimately calls formatIFrameContent
 */
class Control : public UPnPDevice  {
    public:
      Control();
      Control( const char* target);
      virtual ~Control() {}

/**
*   Root based url to the request handler for iFrame content display, registered with the Web server 
*   at setup()
*/
      void               contentPath(char buffer[], size_t size);

/** 
 *  Controls should implement the following methods for display of their Sensor reading:
 *
 *  int    formatContent(char buffer[], int size, int pos)             // HTML Displayed from device target, excluding document header, title, and tail; returns updated write pos
 *  void   frameHeight();                                              // Height of iFrame (defaults to 75)
 *  void   frameWidth();                                               // Width of iFrame (defaults to 300);
 *    
 *
 *  
 *  Controls with complex configutation should implement methods for the following
 *  
 *    configForm(WebContext* svr)              := Presents an HTML form for configuration input. Form submission
 *                                                ultimately calls handleSetConfiguration() on a (derived) Control. Default action only supplies a form 
 *                                                for setting diaplay name. Note: the form "action" attribute is supplied from setConfigurationSvc()->getPath().
 *                                                configForm() is set via setFormHandler() on Control's constructor so is not necessary to set on derived Controls.                                                
 *    handleSetConfiguration(WebContext* svr)  := Request handler for form submission; called from the UPnPService SetConfiguration::handleRequest(). 
 *                                                Default action only sets display name. handleSetConfiguration() is set via setHttpHandler() on 
 *                                                Control's constructor. 
 *    handleGetConfiguration(WebContext* svr)  := Request handler that replies with an XML document describing configuration;
 *                                                called from the UPnPService GetConfiguration::handleRequest(). There is no formal schema for this document 
 *                                                but must start with XML header:
 *                                                    <?xml version=\"1.0\" encoding=\"UTF-8\"?>
 *                                                And must include: 
 *                                                    <config><displayName>display name</displayName>...</config>                      
 *                                                Default action only supplies the displayName attribute. handleGetConfiguration() is set via setHttpHandler() on 
 *                                                Control's constructor.
 */
      int                formatRootContent(char buffer[], int size, int pos);       // Inserts iFrame into RootDeviceDisplay
      virtual int        frameHeight()      {return 75;}
      virtual int        frameWidth()       {return 300;}
      
      SetConfiguration*  setConfigurationSvc()                     {return &_setConfiguration;}                        // Return setConfigutation Service
      GetConfiguration*  getConfigurationSvc()                     {return &_getConfiguration;}                        // Return getConfigutation Service
      virtual void       handleSetConfiguration(WebContext* svr)   {_setConfiguration.handleSetConfiguration(svr);}    // Default form (submit) handler for set configuration
      virtual void       handleGetConfiguration(WebContext* svr)   {_getConfiguration.handleGetConfiguration(svr);}    // Default HTTP handler for get configuration
      virtual void       configForm(WebContext* svr)               {_setConfiguration.configForm(svr);}                // Default form display for set configuration
      void               setup(WebContext* svr);

/**
 *   Display Control content, intended for the endpoint of an iFrame link and
 *   when the Control refreshes its display. Content is supplied thru formatIFrameContent()
 */
      virtual void       displayControl(WebContext* svr);
      void               display(WebContext* svr);

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
      DERIVED_TYPE_CHECK(UPnPDevice);

/**
 *   Copy construction and assignment are not allowed
 */
     DEFINE_EXCLUSIONS(Control);         

      private:
      GetConfiguration     _getConfiguration;
      SetConfiguration     _setConfiguration;
};

} // End of namespace lsc

 #endif
