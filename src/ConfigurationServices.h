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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <UPnPLib.h>
#include <Timer.h>

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

/**
 *   UPnPServices to get and set configutation for a UPnPDevice. A default implementation is provided that allows setting display name, 
 *   but this behavior can be overridden by setting handlers:
 *       SetConfiguration::setHTTPHandler(HandlerFunction h)
 *       SetConfiguration::setFormHandler(HandlerFunction h)
 *       GetConfiguration::setHTTPHandler(HandlerFunction h)
 */
 
class SetConfiguration : public UPnPService {
    public:
    SetConfiguration();
    SetConfiguration(const char* target);
    virtual ~SetConfiguration() {}

    void setFormHandler(HandlerFunction h) {_formHandler = h;}
    void formPath(char buffer[],size_t size);
    void setup(WebContext* svr);
    void handleSetConfiguration(WebContext* svr);                   // Default form (submit) handler for set configuration
    void configForm(WebContext* svr);                               // Default form display for set configuration
    
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
    DERIVED_TYPE_CHECK(UPnPService);

    HandlerFunction      _formHandler = [](WebContext* svr){};

/**
 *   Copy construction and assignment are not allowed
 */
     DEFINE_EXCLUSIONS(SetConfiguration);         

};

class GetConfiguration : public UPnPService {
    public:
    GetConfiguration();
    GetConfiguration(const char* target);
    virtual ~GetConfiguration() {}

    void handleGetConfiguration(WebContext* svr);                   // Default HTTP handler for get configuration

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
    DERIVED_TYPE_CHECK(UPnPService);

/**
 *   Copy construction and assignment are not allowed
 */
     DEFINE_EXCLUSIONS(GetConfiguration);         

};

} // End of namespace lsc

 #endif
