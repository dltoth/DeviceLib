/**
 * 
 */

#ifndef SENSORCONTROLLEDRELAY_H
#define SENSORCONTROLLEDRELAY_H

#include "RelayControl.h"
#include "SoftwareClock.h"

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
 *  it's configuration, and in MANUAL mode it is controlled by the state toggle (ON/OFF) from RelayControl. So, operation 
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
 *    2. void setOutletTimerConfigutation(WebContext*) - for responding to configuration form submission 
 *    3. void getOutletTimerConfiguration(WebContext*) - for responding to requests for configuration
 *    4. void content(char buffer[], int size) - to supply display content
 *    5. ControlState sensorState() - for providing ControlState based on Sensor reading
 */ 

class SensorControlledRelay : public RelayControl {

  public: 
      SensorControlledRelay();
      SensorControlledRelay( const char* type, const char* target );

/**
 *    Relay mode management
 */
      void            setMode(WebContext* svr);                                                           // HttpHandler for setting ControlMode
      UPnPService*    setModeSvc()                  {return &_setModeSvc;}                                // UPnPService for setting ControlMode
      
      boolean         isAUTOMATIC()                 {return(_mode == AUTOMATIC);}                         // Returns TRUE if mode is AUTOMATIC
      boolean         isMANUAL()                    {return(_mode == MANUAL);}                            // Returns TRUE if the mode is MANUAL
      ControlMode     getControlMode()              {return(_mode);}                                      // Returns ControlMode AUTOMATIC/MANUAL
      const char*     controlMode()                 {return((isAUTOMATIC())?("AUTOMATIC"):("MANUAL"));}   // Returns char* representation of ControlMode

/**
 *    Sensor refresh rate
 */
      int             sensorRefresh()                 {return _timer.setPointMillis()/1000;}
      void            sensorRefresh(int secs)         {if( secs > 0 ) _timer.set(secs*1000);}

/**
 *    These methods make explicit the difference between the actual state of the relay and the state as determined
 *    by the Sensor. 
 */
      virtual ControlState  sensorState() = 0;
      ControlState          relayState()                 {return getControlState();}
      void                  relayState(ControlState s)   {RelayControl::setControlState(s);}

/**
*    Frame height from Control
*/
      virtual int      frameHeight()  {return 100;}

/**
 *    Display this Control
 */
      void             content(char buffer[], int size);
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
 *   Macros to define the following Runtime Type Info:
 *     private: static const ClassType  _classType;             
 *     public:  static const ClassType* classType();   
 *     public:  virtual void*           as(const ClassType* t);
 *     public:  virtual boolean         isClassType( const ClassType* t);
 */
      DEFINE_RTTI;
      DERIVED_TYPE_CHECK(RelayControl);

      protected:
      UPnPService      _setModeSvc;
      
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

      SensorControlledRelay(const SensorControlledRelay&)= delete;
      SensorControlledRelay& operator=(const SensorControlledRelay&)= delete;

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