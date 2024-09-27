/**
 * 
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

#include "CustomClock.h"

const char  clock_body[]         PROGMEM = "<p align=\"center\" style=\"font-size:1.35em;\"> %s </p>"
            "<div align=\"center\">"
              "<table>"
              "<tr><td><b>Clock Start:</b></td><td>&ensp;%s</td></tr>"         // Start time
              "<tr><td><b>Running Time:</b></td><td>&ensp;%s</td></tr>"        // Running time
              "</table>"
            "</div><br><br>";

const char  root_clock_body[]    PROGMEM = "<p align=\"center\" style=\"font-size:1.1em;\"> %s </p>";

/**
 *  Static RTT/UPnP type initialization
 */
INITIALIZE_DEVICE_TYPES(CustomClock,LeelanauSoftware-com,CustomClock,1.0.0);

/**
 *  Format CustomClock device HTML into input buffer and return updated write position
 */
int CustomClock::formatContent(char buffer[], int size, int pos) {
  char date[64];
  char start[64];
  char running[64];
  Instant t  = _sysClock.now();                                         // Get system time in timezone set on SystemClock
  Instant s  = _sysClock.startTime().ntpTime().toTimezone(-4.0);        // Get clock start time EST
  t.printDateTime(date,64);                                             // Format date/time into date buffer
  t.printElapsedTime(s, running, 64);                                   // Format running time (difference between start and now) into running buffer
  s.printDateTime(start,64);                                            // Format start time into start buffer
  return formatBuffer_P(buffer,size,pos,clock_body,date,start,running); // Format HTML into buffer and return updated buffer write position
}

int CustomClock::formatRootContent(char buffer[], int size, int pos) {
  char date[64];
  Instant t  = _sysClock.now();                                         // Get system time in timezone set on SystemClock
  t.printDateTime(date,64);                                             // Format date/time into date buffer
  return formatBuffer_P(buffer,size,pos,root_clock_body,date);          // Format HTML into buffer and return updated buffer write position
}

