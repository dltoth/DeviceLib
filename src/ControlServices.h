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
 
#ifndef CONTROLSERVISES_H
#define CONTROLSERVISES_H

#include <UPnPService.h>

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {
  
class SetStateService : public UPnPService {
  public:
    SetStateService() :  UPnPService("setState") {setDisplayName("Set State");};
    SetStateService(const char* target) : UPnPService(target) {setDisplayName("Set State");};

    DEFINE_RTTI;
    DERIVED_TYPE_CHECK(UPnPService);

/**
 *   Copy construction and destruction are not allowed
 */
     DEFINE_EXCLUSIONS(SetStateService);         
};

class SetModeService : public UPnPService {
  public:
    SetModeService() :  UPnPService("setMode") {setDisplayName("Set Mode");};
    SetModeService(const char* target) : UPnPService(target) {setDisplayName("Set Mode");};

    DEFINE_RTTI;
    DERIVED_TYPE_CHECK(UPnPService);

/**
 *   Copy construction and destruction are not allowed
 */
     DEFINE_EXCLUSIONS(SetModeService);         
};

}
#endif