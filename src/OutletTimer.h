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

#ifndef OUTLET_TIMER_H
#define OUTLET_TIMER_H

#include "SoftwareClock.h"
#include "SensorControlledRelay.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

/**
 *   Number of Timer intevals is 2, only because of buffer size
 */
#define MAX_TIMER_INTERVALS 2

/** OutletTimer is a SensorControlledRelay that couples a SoftwareClock (for time) with a relay in order 
 *  to control a power outlet. 
 *
 *  Extending SensorControlledRelay requires implementation of the following:
 *    1. void configForm(WebConext*) - for displaying a configuration form 
 *    2. void setOutletTimerConfigutation(WebContext*) - for responding to configuration form submission 
 *    3. void getOutletTimerConfiguration(WebContext*) - for responding to requests for configuration
 *    4. void content(char buffer[], int size) - to supply display content
 *    5. ControlState sensorState() - for providing ControlState based on Sensor reading
 */ 

class OutletTimer : public SensorControlledRelay {

  public: 
      OutletTimer();
      OutletTimer( const char* target );

      const char*     nextOn();              // Return time (as char*) of next ON cycle

/**
 *    SensorControlledRelay will poll for Sensor state every 5 secs by default, which should not
 *    overwhelm the Thermometer.
 */
      virtual ControlState  sensorState();

/**
*    Frame height from Control
*/
      virtual int      frameHeight()  {return 125;}

/**
 *    Display this Control
 */
      void             content(char buffer[], int size);
      const char*      nextON()       {nextCycle(); return _nextON;}
      const char*      nextOFF()      {nextCycle(); return _nextOFF;}

/**
 *  Set/Get/Check Logging Level provided by RelayControl. Logging Level can be NONE, 
 *  WARNING, INFO, FINE, and FINEST
 */

/**
 *   Configuration support
 */
      void             configForm(WebContext* svr);
      void             setOutletTimerConfiguration(WebContext* svr);
      void             getOutletTimerConfiguration(WebContext* svr); 

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
      DERIVED_TYPE_CHECK(SensorControlledRelay);

      protected:

/**
 *    Timer Control Utilities
 */
      SoftwareClock*  getSoftwareClock()     {return (SoftwareClock*)RootDevice::getDevice(rootDevice(),SoftwareClock::classType());}
      int             getHours(const String& s);               // Returns hours from time string hh:mm
      int             getMinutes(const String& s);             // Returns minutes from time string hh:mm
      int             argSequenceNumber(const String& s);      // Returns web form argument sequence number (for example: START_TIME_2), and -1 on error
      void            nextCycle();                             // Fill nextON and nextOFF buffers based on current time
      void            printInterval(int currentMins, int start, int end, int nON, int nOFF);
/**
 *    Control Variables
 */
      int                 _start[MAX_TIMER_INTERVALS] = {0};        // Time is stored as minutes from midnight
      int                 _end[MAX_TIMER_INTERVALS]   = {0};        // Maximum of MAX_TIMER_INTERVALS start/end intervals allowed, initialized to 0
      char                _nextON[8];                               // Character representation of next ON hh:mm
      char                _nextOFF[8];                              // Character representation of next OFF hh:mm

/**
 *   Copy construction and destruction are not allowed
 */
     DEFINE_EXCLUSIONS(OutletTimer);         

};

} // End of namespace lsc

 #endif