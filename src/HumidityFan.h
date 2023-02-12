/**
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
 *  SensorControlledRelay will poll the Sensor for ControlState, so we also use his as an opportunity to
 *  cache the humidity reading.
 *
 *  Extending SensorControlledRelay requires implementation of the following:
 *    1. void configForm(WebConext*) - for displaying a configuration form 
 *    2. void setOutletTimerConfigutation(WebContext*) - for responding to configuration form submission 
 *    3. void getOutletTimerConfiguration(WebContext*) - for responding to requests for configuration
 *    4. void content(char buffer[], int size) - to supply display content
 *    5. ControlState sensorState() - for providing ControlState based on Sensor reading
 */ 

class HumidityFan : public SensorControlledRelay {

  public: 
      HumidityFan();
      HumidityFan( const char* type, const char* target );

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
      void             content(char buffer[], int size);

/**
 *  Set/Get/Check Logging Level provided by RelayControl. Logging Level can be NONE, 
 *  WARNING, INFO, FINE, and FINEST
 */

/**
 *   Configuration support
 */
      void             configForm(WebContext* svr);
      void             setHumidityFanConfiguration(WebContext* svr);
      void             getHumidityFanConfiguration(WebContext* svr);  

/**
 *   Macros to define the following Runtime Type Info:
 *     private: static const ClassType  _classType;             
 *     public:  static const ClassType* classType();   
 *     public:  virtual void*           as(const ClassType* t);
 *     public:  virtual boolean         isClassType( const ClassType* t);
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

      HumidityFan(const HumidityFan&)= delete;
      HumidityFan& operator=(const HumidityFan&)= delete;

};

} // End of namespace lsc

 #endif