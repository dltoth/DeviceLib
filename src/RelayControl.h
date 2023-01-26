
/**
 * 
 */

#ifndef RELAYCONTROL_H
#define RELAYCONTROL_H

#include <CommonProgmem.h>
#include <Control.h>

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

/** RelayControl is a Control for managing a relay, which in turn could control an outlet. RelayControl has 2 states ON, and OFF controlled by a toggle 
 *  in the HTML interface. Switch ControlState is read from the relay itself rather than managing an internal variable.
 *  Implementers of this class must provide:
 *      virtual void  content(WebContext* svr);    // From Control - displays the device based on RelayControl state
 *      virtual void  setState(WebContext* svr);   // HttpHandler for retrieving arguments and setting the RelayControl state
 *  The actual electronics for  ON/OFF are implemented in the method setOutletState(ControlState)
 *  RelayControl relay uses WeMOS pin D5 (GPIO pin 14), but pin can be set via:
 *      void setPin(int pin);
 *  Configuration support is provided by Control allowing  deviceName definition 
 */

class RelayControl : public Control {

  public: 
      RelayControl();
      RelayControl( const char* type, const char* target );

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
      void             content(WebContext* svr);
      void             setup(WebContext* svr);
 
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
      UPnPService         _setStateSvc;

/**
 *    Control Variables
 */
      int                 _pin          = WEMOS_D5;                 // Pin default is D5 (GPIO pin 14)
      LoggingLevel        _logging      = NONE;
      
      RelayControl(const RelayControl&)= delete;
      RelayControl& operator=(const RelayControl&)= delete;

};

} // End of namespace lsc

 #endif
