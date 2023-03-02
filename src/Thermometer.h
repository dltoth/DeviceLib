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

#ifndef THERMOMETER_H
#define THERMOMETER_H
#include <SensorDevice.h>
#include <DHTesp.h>

#define CELCIUS    'C'
#define FAHRENHEIT 'F'

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {
 
  
class GetTempHum : public UPnPService {
  public:
  GetTempHum();
  void handleRequest(WebContext* svr);  

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
    DERIVED_TYPE_CHECK(UPnPService);

/**
 *   Copy construction and destruction are not allowed
 */
     DEFINE_EXCLUSIONS(GetTempHum);         
};

/**
 *   DHT22 Thermometer Device. 
 *   Temperature and Humidity readings query the DHT22 directly so should not exceed 1 second intervals. Readings in a tight loop should be cached
 *   and refreshed with a Timer. For example, if Timer _t is a member of a class caching temperature and humidity, define a member function:
 *      void timerCallback() {
 *           updateCache();
 *           _t.reset();
 *           _t.start();
 *      }
 *   and set the Timer callback and refresh interval in the Class constructor:
 *      _t.set(2000)
 *      _t.setHandler([this]{this->timerCallback();});
 *
 */
class Thermometer : public Sensor {
  public:
  Thermometer();
  Thermometer(const char* target);


  float           temp();
  float           hum();
  void            setFahrenheit();
  void            setCelcius();
  char            unit();
  int             pin();
  void            pin(int p);

  static boolean  isFahrenheit(char u) {return ((u==FAHRENHEIT)?(true):(false));}
  static boolean  isCelcius(char u)    {return ((u==CELCIUS)?(true):(false));}
  boolean         isFahrenheit()       {return isFahrenheit(_unit);}
  boolean         isCelcius()          {return isCelcius(_unit);}
  
/**
 *   Virtual Functions required for UPnPDevice
 */
  void            setup(WebContext* svr);

/**
 *   Required by Sensor
 */
   void           content(char buffer[], int bufferSize);

/**
 *   Configuration support
 */
   void           configForm(WebContext* svr);
   void           setThermometerConfiguration(WebContext* svr);
   void           getThermometerConfiguration(WebContext* svr);  

/**
 *   Macros to define the following Runtime Type Info:
 *     private: static const ClassType  _classType;             
 *     public:  static const ClassType* classType();   
 *     public:  virtual void*           as(const ClassType* t);
 *     public:  virtual boolean         isClassType( const ClassType* t);
 */
    DEFINE_RTTI;
    DERIVED_TYPE_CHECK(Sensor);
  
  private:
  GetTempHum      _getTempHum;
  DHTesp          _dht;
  int             _pin = WEMOS_D2;        // Set pin to GPIO 4, or WeMOS D2
  char            _unit = FAHRENHEIT;
  
/**
 *   Copy construction and destruction are not allowed
 */
     DEFINE_EXCLUSIONS(Thermometer);         
};

} // End of namespace lsc

 #endif
