/**
 * 
 *  UPnPDevice Library
 *  Copyright (C) 2024  Daniel L Toth
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

#include "ClockDevice.h"

/**
 *   clock_body will provide HTML at the device location, including date/time, start time, and running time
 */
const char  clock_body[]         PROGMEM = "<p align=\"center\" style=\"font-size:1.25em;\"> %s </p>"  // date/time
            "<div align=\"center\">"
              "<table>"
              "<tr><td><b>Clock Start:</b></td><td>&ensp;%s</td></tr>"                                 // Start time
              "<tr><td><b>Running Time:</b></td><td>&ensp;%s</td></tr>"                                // Running time
              "</table>"
            "</div><br><br>";

/**
 *   root_body will provide HTML at the root location, including only date/time
 */
const char  root_body[]   PROGMEM = "<p align=\"center\" style=\"font-size:1.25em;\"> %s </p><br>";

INITIALIZE_DEVICE_TYPES(ClockDevice,LeelanauSoftware-com,ClockDevice,1.0.0);

int ClockDevice::formatRootContent(char buffer[], int size, int pos) {
  Instant t = _clock.now();
  char buff[64];
  t.printDateTime(buff,64);
  int result = formatBuffer_P(buffer,size,pos,root_body,buff);
  return result;
}

int ClockDevice::formatContent(char buffer[], int size, int pos) {
  Instant t = _clock.now();
  char date[64];
  char running[64];
  char start[64];
  Instant s  = _clock.startTime().ntpTime().toTimezone(-5.0);
  t.printDateTime(date,64);
  t.printElapsedTime(s, running, 64);
  s.printDateTime(start,64);
  int result = formatBuffer_P(buffer,size,pos,clock_body,date,start,running);
  return result;
}

void ClockDevice::setup(WebContext *svr) {
  Sensor::setup(svr);
  _clock.now();
}
