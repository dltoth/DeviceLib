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

#include "ControlServices.h"

namespace lsc {
INITIALIZE_STATIC_TYPE(SetStateService);
INITIALIZE_UPnP_TYPE(SetStateService,urn:LeelanauSoftware-com:service:setState:1);
}

namespace lsc {
INITIALIZE_STATIC_TYPE(SetModeService);
INITIALIZE_UPnP_TYPE(SetModeService,urn:LeelanauSoftware-com:service:setMode:1);
}

