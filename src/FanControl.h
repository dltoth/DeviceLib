
/**
 * 
 */

#ifndef FANCONTROL_H
#define FANCONTROL_H

#include <CommonProgmem.h>
#include <Control.h>
#include "Thermometer.h"
#include "SoftwareClock.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

/**
 *   Minimum humidity refresh in seconds
 */ 
#define MIN_REFRESH 1

/** FanControl is a Control for managing a Vent Fan with a Humidity Sensor. This Device assumes the presence of a Thermometer Sensor as a sibling to this
 *  Control. FanControl has 2 modes: ATOMATIC and MANUAL, and the fan has 2 states ON, and OFF. In AUTOMATIC mode, the fan is controlled by a
 *  Humidity Sensor (Thermometer), and in Manual Mode, the fan can be turned ON and OFF manually.
 *  Implementers of this class must provide:
 *      virtual void  content(WebContext* svr);   // From Control - displays the device based on mode and fan state
 *      virtual void  setMode(WebContext* svr);   // HttpHandler for retrieving arguments and setting the fan mode
 *  The actual electronics for fan ON/OFF are implemented in the method setControlState(ControlState)
 *  FanControl sets up a Timer to read Humidity from the Sensor at most once per second, caching the result in the member variable: 
 *      float _humidity;
 *  FanControl relay uses WeMOS pin D4 (GPIO pin 2), but pin can be set via:
 *      void setPin(int pin);
 *      
 */
class FanControl : public Control {

  public: 
      FanControl();
      FanControl( const char* type, const char* target );

      virtual int     frameHeight()  {return 150;}                                 // Frame height from Control

/**
 *    ControlMode management
 */
      boolean         isMode(ControlMode mode)    {return (mode == _mode);}        // Check for FanControl mode
      void            setMode(ControlMode mode)   {_mode = mode;}                  // Sets Control mode AUTOMATIC/MANUAL
      ControlMode     getControlMode()            {return _mode;}                  // Return Control mode AUTOMATIC/MANUAL
      const char*     controlMode()               {return((getControlMode()==AUTOMATIC)?("AUTOMATIC"):("MANUAL"));}
      UPnPService*    setModeSvc()                {return &_setModeSvc;}           // UPnPService for setting ControlMode
      void            setMode(WebContext* svr);                                    // HttpHandler for setting ControlMode

/**
 *    Threshold management
 */
      int             getThreshold()                 {return _threshold;}             // Humidity threshold for turning on the Vent Fan (1 to 99)
      float           getHumidity()                  {return _humidity;}              // Return cached humidity
      void            checkThreshold();                                               // Check humidity against threshold, potentially turning the fan ON
      void            setThreshold(int value)        {if( (value > 0) && (value < 100) )  _threshold = value;}
      void            updateHumidity()               {Thermometer* t = getThermometer();if( t != NULL ) _humidity = t->hum();}

/**
 *    Fan state management
 */
      boolean         fanIsON()                      {return(getControlState() == ON);}   // Returns TRUE if the Fan is ON
      boolean         fanIsOFF()                     {return(getControlState() == OFF);}  // Returns TRUE if the Fan is OFF
      ControlState    getControlState()              {return((digitalRead(getPin()) == HIGH)?(ON):(OFF));}
      const char*     controlState()                 {return((fanIsON())?("ON"):("OFF"));}
      void            setPin(int pin)                {_pin = ((pin>=0)?((pin<17)?(pin):(16)):(0));}
      int             getPin()                       {return _pin;}
      int             getHumRefresh()                {return _humRefresh;}                                          // Return humidity refresh in seconds
      void            setHumRefresh(int h)           {if( h > MIN_REFRESH ) {_humRefresh=h;_timer.set(1000*h);} }   // Set humidity refresh in seconds

/**
 *   If state is AUTOMATIC, check Humidity Sensor and turn fan ON or OFF.
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
      void             setFanControlConfiguration(WebContext* svr);
      void             getFanControlConfiguration(WebContext* svr);  

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
      void         setControlState(ControlState flag);    
      Thermometer* getThermometer();                              // Returns a non-NULL Thermometer if both FanControl and Thermometer have been added to the RootDevice
      
      UPnPService         _setModeSvc;

/**
 *    Control Variables
 */
      int                 _threshold   = 75;                       // Default threshold set to 75% humidity
      float               _humidity    = 0.0;                      // Last humidity reading
      int                 _humRefresh  = MIN_REFRESH;              // Humidity refresh defaults to 1 second
      ControlMode         _mode        = MANUAL;                   // Control mode AUTOMATIC/MANUAL
      int                 _pin         = WEMOS_D5;                 // Pin default is D5 (GPIO pin 14)
      LoggingLevel        _logging     = NONE;
      Timer               _timer;                                  // Set up a Timer to read Humidity only once every _humRefresh seconds, 
                                                                   // caching the result in _humidity 
      
      FanControl(const FanControl&)= delete;
      FanControl& operator=(const FanControl&)= delete;

      void timerCallback() {
         updateHumidity();
         this->checkThreshold();
         _timer.reset();
         _timer.start();
      }

};

} // End of namespace lsc

 #endif
