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

#ifndef HYDROMETER_H
#define HYDROMETER_H

#include "SensorDevice.h"

#define ANALOG_WATER  400
#define ANALOG_AIR    900

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {
  
class GetSoilMoisture : public UPnPService {
  public:
  GetSoilMoisture();
  virtual ~GetSoilMoisture() {}

  void handleRequest(WebContext* svr);  

/**
 *   Macros to define the following Runtime Type Info:
 *     private: static const ClassType  _classType;             
 *     public:  static const ClassType* classType();   
 *     public:  virtual void*           as(const ClassType* t);
 *     public:  virtual boolean         isClassType( const ClassType* t);
 */
    DEFINE_RTTI;
    DERIVED_TYPE_CHECK(UPnPService);

/**
 *   Copy construction and assignment are not allowed
 */
     DEFINE_EXCLUSIONS(GetSoilMoisture);         
};

/**
 *   Hydrometer is a Sensor that reads from the analog pin and computes soil moisture content.
 */
class Hydrometer : public Sensor {
  public:
  Hydrometer();
  Hydrometer(const char* target);
  virtual ~Hydrometer() {}

  float      soilMoisture();
  float      soilMoisture( int ar );
  int        pin();
  int        water()       {return _water;}
  void       water( int w) {_water = (((w>0)&(w<1000))?(w):(_water));}
  int        air()         {return _air;}
  void       air( int a)   {_air = (((a>0)&(a<1000))?(a):(_air));}  
  void       pin(int p);
  
/**
 *   Virtual Functions required for UPnPDevice
 */
  void       setup(WebContext* svr);
  
/**
 *   Virtual Functions required by Sensor, using default configuration
 */
   int       formatContent(char buffer[], int bufferSize, int pos);
   int       formatRootContent(char buffer[], int bufferSize, int pos);

/** Configuration support.
 *  
 */
   void      configForm(WebContext* svr);
   void      handleSetConfiguration(WebContext* svr);
   void      handleGetConfiguration(WebContext* svr);  
   void      acquireWet(WebContext* svr);  
   void      acquireDry(WebContext* svr);  
 
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
    DERIVED_TYPE_CHECK(Sensor);
  
  private:
  GetSoilMoisture   _getSoilMoisture;
  int               _pin = A0;
  int               _air = ANALOG_AIR;
  int               _water = ANALOG_WATER;
  int               _acquireWet = -1;
  int               _acquireDry = -1;

  void  displayForm(WebContext* svr, int acquireDry, int acquireWet );

/**
 *   Copy construction and assignment are not allowed
 */
     DEFINE_EXCLUSIONS(Hydrometer);         

};

} // End of namespace lsc

 #endif
