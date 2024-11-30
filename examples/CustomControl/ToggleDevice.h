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

#ifndef TOGGLEDEVICE_H
#define TOGGLEDEVICE_H

#include <DeviceLib.h>

#define MSG_SIZE 64

/** ToggleDevice is a Control that manages 2 states ON, and OFF controlled by a toggle in the HTML interface.
 *  Implementation supplies the following methods:
 *      virtual void  formatContent(WebContext* svr);    // From Control - displays the toggle based on ToggleDevice state
 *      virtual void  setState(WebContext* svr);         // HttpHandler for retrieving arguments and setting the ToggleDevice state
 *  Configuration support allows  deviceName definition and Control message
 */

class ToggleDevice : public Control {

  public: 
      ToggleDevice() : Control("toggle") {setDisplayName("Toggle Device");}

      ToggleDevice( const char* target ) : Control(target) {setDisplayName("Toggle Device");}

      virtual int     frameHeight()  {return 75;}       // Frame height from Control

/**
 *    ToggleDevice state management
 */
      void            setState(WebContext* svr);                                                        // HttpHandler for setting ControlState
      
      boolean         isON()                      {return(getState() == ON);}                           // Returns TRUE if the relay is ON
      boolean         isOFF()                     {return(getState() == OFF);}                          // Returns TRUE if the relay is OFF
      ControlState    getState()                  {return(_state);}                                     // Returns State ON/OFF
      const char*     state()                     {return((isON())?("ON"):("OFF"));}                    // Returns char* representation of State

/**
 *    Display this Control
 */
      int             formatContent(char buffer[], int size, int pos);
      void            setup(WebContext* svr);
 
      DEFINE_RTTI;
      DERIVED_TYPE_CHECK(Control);

      protected:
      void            setState(ControlState flag) {_state = flag;}            

/**
 *    Control Variables
 */
      ControlState         _state = OFF;

      DEFINE_EXCLUSIONS(ToggleDevice);         

};

 #endif
