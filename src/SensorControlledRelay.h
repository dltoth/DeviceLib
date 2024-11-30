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

#ifndef SENSORCONTROLLEDRELAY_H
#define SENSORCONTROLLEDRELAY_H

#include "RelayControl.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

/**
 *   How often to poll the Sensor (in seconds)
 */
#define SENSOR_REFRESH 5

/** SensorControlledRelay is a RelayControl that can be controlled by a Sensor. It uses the two relay states ON/OFF from 
 *  RelayControl and adds two modes, AUTOMATIC and MANUAL. In AUTOMATIC mode the device is controlled by the Sensor and
 *  its configuration, and in MANUAL mode it is controlled by the state toggle (ON/OFF) from RelayControl. So, operation 
 *  is determined by two toggles, one for mode (AUTOMATIC/MANUAL) and one for state (ON/OFF). Tripping the state toggle 
 *  also trips the mode toggle.
 *  
 *  Relay state management, pin definition, and logging all come from RelayControl and a new UPnPService is added to set
 *  ControlMode. SensorControlledRelay is a virtual base class deriving from RelayControl, and as such, subclasses must 
 *  supply implementation for the following method:
 *     ControlState sensorState() := Returns ControlState according to the Sensor
 *  Note that in the semantics below, sensorState refers to the ControlState that the Sensor indicates, and relayState 
 *  refers to the actual ControlState of the Relay as determined by reading pin. These may be different in the case of
 *  MANUAL operation.
 *
 *  Extending SensorControlledRelay requires implementation of the following:
 *    1. void configForm(WebConext*) - for displaying a configuration form 
 *    2. void setControlConfigutation(WebContext*) - for responding to configuration form submission 
 *    3. void getControlConfiguration(WebContext*) - for responding to requests for configuration
 *    4. int  formatContent(char buffer[], int size, int pos); - to supply display content
 *    5. ControlState sensorState() - for providing ControlState based on Sensor reading
 *
 *  Note that the formatContent() method should also rely on SensorControlledRelay::formatContent() to supply the standard toggles,
 *  state management, and mode management. Something like:
 *     void  MyControl::content(char buffer[], int size) {  
 *              int pos = SensorControlledRelay::formatContent(buffer,size);
 *              pos = formatBuffer_P(buffer,size,pos,...); 
 *              ...         
 *     } 
 */ 

class SensorControlledRelay : public RelayControl {

  public: 
      SensorControlledRelay();
      SensorControlledRelay( const char* target );
      virtual ~SensorControlledRelay() {}

/**
 *    Relay mode management
 */
      void            setMode(WebContext* svr);                                                           // HttpHandler for setting ControlMode
      SetModeService* setModeSvc()                  {return &_setModeSvc;}                                // UPnPService for setting ControlMode
      
      boolean         isAUTOMATIC()                 {return(_mode == AUTOMATIC);}                         // Returns TRUE if mode is AUTOMATIC
      boolean         isMANUAL()                    {return(_mode == MANUAL);}                            // Returns TRUE if the mode is MANUAL
      ControlMode     getControlMode()              {return(_mode);}                                      // Returns ControlMode AUTOMATIC/MANUAL
      const char*     controlMode()                 {return((isAUTOMATIC())?("AUTOMATIC"):("MANUAL"));}   // Returns char* representation of ControlMode

/**
 *    Sensor refresh rate (polling interval)
 */
      int             sensorRefresh()               {return _timer.setPointMillis()/1000;}
      void            sensorRefresh(int secs)       {if( secs > 0 ) _timer.set(secs*1000);}

/**
 *    These methods make explicit the difference between the actual state of the relay and the state as determined
 *    by the Sensor. 
 */
      virtual ControlState  sensorState() = 0;
      ControlState          relayState()                 {return getControlState();}            // From RelayControl
      void                  relayState(ControlState s)   {RelayControl::setControlState(s);}    // Set relay state via RelayControl

/**
*    Frame height from Control
*/
      virtual int      frameHeight()  {return 100;}

/**
 *    Display this Control
 */
      int              formatContent(char buffer[], int size, int pos);
      void             setup(WebContext* svr);

/**
 *  Set/Get/Check Logging Level provided by RelayControl. Logging Level can be NONE, 
 *  WARNING, INFO, FINE, and FINEST
 */

/**
 *    Update the device
 */
      void             doDevice();
 
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
      DERIVED_TYPE_CHECK(RelayControl);

      protected:
      SetModeService   _setModeSvc;
      
      void             setControlState(ControlState s);
      void             setControlMode(ControlMode mode); 
      void             lastSensorState(ControlState s)    {_sensorState = s;}
      ControlState     lastSensorState()                  {return _sensorState;}
      bool             sensorStateChange(ControlState s)  {return(s == _sensorState);}
      bool             isRelayState(ControlState s)       {return s == relayState();}

/**
 *    Control Variables
 */
      ControlMode         _mode          = MANUAL;          // AUTOMATIC/MANUAL triggered by the toggle
      ControlState        _sensorState   = OFF;             // Last measured ControlState by Sensor
      Timer               _timer;

/**
 *   Copy construction and assignment are not allowed
 */
     DEFINE_EXCLUSIONS(SensorControlledRelay);         

/**
 *    Timer callback sets relay state according to the Sensor unless the toggle has been triggered,
 *    as indicated by mode having been set to MANUAL. Sensor will not effect a change to ControlState
 *    unless mode is AUTOMATIC.
 */
      void timerCallback() {
        ControlState state = sensorState(); 
        if(sensorStateChange(state))  lastSensorState(state);
/**
 *      RelayState is set ONLY if it's different from the actual relay, and mode is AUTOMATIC.
 */
        if( !isRelayState(state) && isAUTOMATIC() ) RelayControl::setControlState(state);
        _timer.reset();
        _timer.start();
      }

};

} // End of namespace lsc

 #endif