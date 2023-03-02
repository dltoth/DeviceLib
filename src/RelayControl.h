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

#ifndef RELAYCONTROL_H
#define RELAYCONTROL_H

#include <CommonProgmem.h>
#include <Control.h>
#include "ControlServices.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

/** RelayControl is a Control for managing a relay, which in turn could control an outlet. RelayControl has 2 states ON, and OFF controlled by a toggle 
 *  in the HTML interface. Relay ControlState is read from the relay itself rather than managing an internal variable.
 *  RelayControl publishes a UPnPService for setting relay state (setState) as implemented by the member variable 
 *      SetStateService    _setStateSvc;
 *  Subclasses should provide:
 *      virtual void  content(WebContext* svr);    // From Control - displays the device based on RelayControl state
 *      virtual void  setState(WebContext* svr);   // HttpHandler for the set state service that retrieves arguments 
 *                                                    and sets RelayControl state
 *  The actual electronics for  ON/OFF are implemented in the method setControlState(ControlState)
 *  RelayControl relay uses WeMOS pin D5 (GPIO pin 14), but pin can be set via:
 *      void setPin(int pin);
 *  Configuration support is provided by Control allowing  deviceName definition
 */

class RelayControl : public Control {

  public: 
      RelayControl();
      RelayControl( const char* target );

      virtual int     frameHeight()  {return 100;}                                                      // Frame height from Control

/**
 *    Relay state management
 */
      void            setState(WebContext* svr);                                                        // HttpHandler for setting ControlState
      UPnPService*    setStateSvc()               {return &_setStateSvc;}                               // UPnPService for setting ControlState
      
      boolean         isON()                      {return(getControlState() == ON);}                    // Returns TRUE if the relay is ON
      boolean         isOFF()                     {return(getControlState() == OFF);}                   // Returns TRUE if the relay is OFF
      ControlState    getControlState()           {return((digitalRead(getPin()) == HIGH)?(ON):(OFF));} // Returns ControlState ON/OFF
      const char*     controlState()              {return((isON())?("ON"):("OFF"));}                    // Returns char* representation of ControlState

/**
 *    Relay Pin definition
 */
      void            setPin(int pin)             {_pin = ((pin>=0)?((pin<17)?(pin):(16)):(0));}
      int             getPin()                    {return _pin;}

/**
 *    Display this Control
 */
      void             content(char buffer[], int size);
      void             setup(WebContext* svr);
 
/**
 *  Set/Get/Check Logging Level. Logging Level can be NONE, WARNING, INFO, FINE, and FINEST
 */
  void                 logging(LoggingLevel level)             {_logging = level;}
  LoggingLevel         logging()                               {return _logging;}
  boolean              loggingLevel(LoggingLevel level)        {return(logging() >= level);}

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
      DERIVED_TYPE_CHECK(Control);

      protected:
      virtual void        setControlState(ControlState flag);            
      SetStateService     _setStateSvc;

/**
 *    Control Variables
 */
      int                 _pin          = WEMOS_D5;                 // Pin default is D5 (GPIO pin 14)
      LoggingLevel        _logging      = NONE;
      
/**
 *   Copy construction and destruction are not allowed
 */
     DEFINE_EXCLUSIONS(RelayControl);         

};

} // End of namespace lsc

 #endif
