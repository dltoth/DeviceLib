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

#ifndef HUMIDITY_FAN_H
#define HUMIDITY_FAN_H

#include "Thermometer.h"
#include "SensorControlledRelay.h"

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

/**
 *   Default humidity threshold for humidity fan is 75%
 */
#define THRESHOLD 75

/** HumidityFan is a SensorControlledRelay that couples a Thermometer (for humidity) with a relay in order 
 *  to control a humidity fan. 
 *  SensorControlledRelay will poll the Sensor for ControlState, so we also use this as an opportunity to
 *  cache the humidity reading.
 *
 *  Extending SensorControlledRelay requires implementation of the following:
 *    1. void configForm(WebConext*) - for displaying a configuration form 
 *    2. void setOutletTimerConfigutation(WebContext*) - for responding to configuration form submission 
 *    3. void getOutletTimerConfiguration(WebContext*) - for responding to requests for configuration
 *    4. int  formatContent(char buffer[], int size, int pos); - to supply display content
 *    5. ControlState sensorState() - for providing ControlState based on Sensor reading
 */ 

class HumidityFan : public SensorControlledRelay {

  public: 
      HumidityFan();
      HumidityFan( const char* target );
      virtual ~HumidityFan() {}

      float            humidity()                      {return _humidity;}
      int              threshold()                     {return _threshold;}
      void             threshold(int threshold)        {_threshold = ((threshold>0)?(threshold):(_threshold));}

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
      int             formatContent(char buffer[], int size, int pos);

/**
 *  Set/Get/Check Logging Level provided by RelayControl. Logging Level can be NONE, 
 *  WARNING, INFO, FINE, and FINEST
 */

/**
 *   Configuration support (set from Control)
 */
      void             configForm(WebContext* svr);                 // Config form display
      void             handleSetConfiguration(WebContext* svr);     // HTTP handler for setConfiguration Service - set on configForm
      void             handleGetConfiguration(WebContext* svr);     // HTTP Handler for getConfiguration Service

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
 *    Humidity is cached every polling cycle so if we want it more often the Thermometer is not overwhelmed
 */
      void          humidity(float h)    {_humidity = ((h > 0)?(_humidity=h):(_humidity));}
      Thermometer*  getThermometer()     {return (Thermometer*)RootDevice::getDevice(rootDevice(),Thermometer::classType());}

/**
 *    Control Variables
 */
      int            _threshold = THRESHOLD;
      float          _humidity  = 0.0;

/**
 *   Copy construction and assignment are not allowed
 */
     DEFINE_EXCLUSIONS(HumidityFan);         

};

} // End of namespace lsc

 #endif