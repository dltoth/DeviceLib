
/**
 * 
 */

#ifndef TIMERCONTROL_H
#define TIMERCONTROL_H

#include <CommonProgmem.h>
#include <Control.h>
#include "SoftwareClock.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

/** TimerControl is a Control for managing a power outlet. This Device assumes the presence of a SoftwareClock Sensor as a sibling to this
 *  Control. TimerControl has 2 modes: ATOMATIC and MANUAL, and the outlet has 2 states ON, and OFF. In AUTOMATIC mode, 
 *  the outlet is controlled by up to 3 time intervals, and in Manual Mode, the outlet can be turned ON and OFF manually.
 *  A Timer is set up to check current time against each of the control intervals every INTERVAL_CHECK seconds. If the Control is in AUTOMATIC mode, outlets are 
 *  adjusted accordingly.
 *  Implementers of this class must provide:
 *      virtual void  content(WebContext* svr);   // From Control - displays the device based on mode and outlet state
 *      virtual void  setMode(WebContext* svr);   // HttpHandler for retrieving arguments and setting the outlet mode
 *  The actual electronics for outlet ON/OFF are implemented in the method setOutletState(ControlState)
 *  TimerControl relay uses WeMOS pin D5 (GPIO pin 14), but pin can be set via:
 *      void setPin(int pin);
 *      
 */

#define MAX_TIMER_INTERVALS 3
#define INTERVAL_CHECK      15             // Interval check in seconds

class TimerControl : public Control {

  public: 
      TimerControl();
      TimerControl( const char* type, const char* target );

      virtual int     frameHeight()  {return 150;}                                    // Frame height from Control

/**
 *    TimerControlMode management
 */
      boolean         isMode(ControlMode mode)    {return (mode == _mode);}        // Check for FanControl mode
      void            setMode(ControlMode mode)   {_mode = mode;}                  // Sets Control mode AUTOMATIC/MANUAL
      ControlMode     getControlMode()            {return _mode;}                  // Return Control mode AUTOMATIC/MANUAL
      const char*     controlMode()               {return((getControlMode()==AUTOMATIC)?("AUTOMATIC"):("MANUAL"));}
      UPnPService*    setModeSvc()                {return &_setModeSvc;}           // UPnPService for setting ControlMode
      void            setMode(WebContext* svr);                                    // HttpHandler for setting ControlMode

/**
 *    Outlet state management
 */
      boolean         outletIsON()                {return(getControlState() == ON);}                    // Returns TRUE if the outlet is ON
      boolean         outletIsOFF()               {return(getControlState() == OFF);}                   // Returns TRUE if the outlet is OFF
      ControlState    getControlState()           {return((digitalRead(getPin()) == HIGH)?(ON):(OFF));} // Returns ControlState ON/OFF
      const char*     controlState()              {return((outletIsON())?("ON"):("OFF"));}              // Returns char* representation of ControlState
      void            setPin(int pin)             {_pin = ((pin>=0)?((pin<17)?(pin):(16)):(0));}
      int             getPin()                    {return _pin;}
      const char*     nextOn();                                                                         // Return time (as char*) of next ON cycle
/**
 *    Check current time against the timer intervals and turn outlet ON or OFF.
 */
      void            checkIntervals();

/**
 *   If state is AUTOMATIC, check time against timer interval and turn ON or OFF.
 *   Called from RootDevce::doDevice() 
 */ 
      void             doDevice();

/**
 *    Display this Control
 */
      void             content(WebContext* svr);
      void             setup(WebContext* svr);

/**
 *   Configuration support
 */
      void             configForm(WebContext* svr);
      void             setTimerControlConfiguration(WebContext* svr);
      void             getTimerControlConfiguration(WebContext* svr);  

/**
 *  Set/Get/Check Logging Level. Logging Level can be NONE, WARNING, INFO, FINE, and FINEST
 */
  void                 logging(LoggingLevel level)             {_logging = level;}
  LoggingLevel         logging()                               {return _logging;}
  boolean              loggingLevel(LoggingLevel level)        {return(logging() >= level);}

/**
 *   Macros to define the following Runtime Type Info:
 *     private: static const ClassType  _classType;             
 *     public:  static const ClassType* classType();   
 *     public:  virtual void*           as(const ClassType* t);
 *     public:  virtual boolean         isClassType( const ClassType* t);
 */
      DEFINE_RTTI;
      DERIVED_TYPE_CHECK(Control);

      protected:
      void                setControlState(ControlState flag);
      SoftwareClock*      getSoftwareClock();                      // Returns a non-NULL SoftwareClock if a one has been added to the RootDevice
      int                 getHours(const String& s);               // Returns hours from time string hh:mm
      int                 getMinutes(const String& s);             // Returns minutes from time string hh:mm
      int                 argSequenceNumber(const String& s);      // Returns web form argument sequence number (for example: START_TIME_2), and -1 on error
            
      UPnPService         _setModeSvc;

/**
 *    Control Variables
 */
      ControlMode         _mode         = MANUAL;                   // Control mode AUTOMATIC/MANUAL
      int                 _pin          = WEMOS_D5;                 // Pin default is D5 (GPIO pin 14)
      int                 _start[MAX_TIMER_INTERVALS];              // Time is stored as minutes from midnight
      int                 _end[MAX_TIMER_INTERVALS];                // Maximum of MAX_TIMER_INTERVALS start/end intervals allowed, initialized to 0 in constructor
      char                _nextON[8];                               // Character representation of next ON cycle hh:mm
      LoggingLevel        _logging     = NONE;
      Timer               _timer;                                  // Set up a Timer to check against time intervals every INTERVAL_CHECK seconds
      
      TimerControl(const TimerControl&)= delete;
      TimerControl& operator=(const TimerControl&)= delete;

      void timerCallback() {
         this->checkIntervals();
         _timer.reset();
         _timer.start();
      }

};

} // End of namespace lsc

 #endif
