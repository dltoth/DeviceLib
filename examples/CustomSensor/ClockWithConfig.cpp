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

#include "ClockWithConfig.h"

/**
 *   Template to format XML for return of configuration; consists of display name and timezone
 */
const char ClockDevice_get_config[]  PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                            "<config>"
                                               "<displayName>%s</displayName>"
                                               "<timeZone>%d</timeZome>"
                                            "</config>";

/**
 *   Template for the form handler to set configuration. Allows input for display name and timezone.
 */
const char ClockDevice_set_config[]  PROGMEM = "<form action=\"%s\"><div align=\"center\">"
            "<br><div align=\"center\">"
              "<table>"
              "<tr><td><b><label for=\"displayName\">Sensor Name</label></b></td>"
                  "<td>&ensp;<input type=\"text\" placeholder=\"%s\" size=\"16\" name=\"displayName\"></td></tr>"    // Device display name    
              "<tr><td><b><label for=\"tz\">Timezone</label></b></td>"
                  "<td>&ensp;<input type=\"text\" name=\"tz\" placeholder=\"%.2f\"  ></td></tr>"                     // Timezone
              "</table><br><br>"
            "</div>"
            "<button class=\"fmButton\" type=\"submit\">Submit</button>&nbsp&nbsp"
            "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button>"  // Cancel path
            "</div></form>";

INITIALIZE_DEVICE_TYPES(ClockWithConfig,LeelanauSoftware-com,ClockWithConfig,1.0.0);

void ClockWithConfig::configForm(WebContext* svr) {
  char buffer[1000];
  int size = sizeof(buffer);
  
/**
 *    Config Form HTML Start with Title
 */
  int pos = formatHeader(buffer,size,"Clock Configuration");

/**
 *  Config Form Content
 */
  char pathBuff[100];
  getPath(pathBuff,100);
  char svcPath[100];
  setConfigurationSvc()->getPath(svcPath,100);
  pos = formatBuffer_P(buffer,size,pos,ClockDevice_set_config,svcPath,getDisplayName(),getTimezone(),pathBuff);

/**
 *  Config Form HTML Tail
 */
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer); 
}

void ClockWithConfig::handleSetConfiguration(WebContext* svr) {
  int numArgs = svr->argCount();
  for( int i=0; i<numArgs; i++) {
     const String& argName = svr->argName(i);
     const String& arg = svr->arg(i);
     if( argName.equalsIgnoreCase("TZ") ) {
        double tz = 0.0;
        if(arg.startsWith("-")) tz = -(arg.substring(1).toDouble());
        else tz = arg.toDouble();
        setTimezone(tz);
     }
     else if( argName.equalsIgnoreCase("DISPLAYNAME") ) {if( arg.length() > 0 ) setDisplayName(arg.c_str());}
  }
  display(svr);  
}

void ClockWithConfig::handleGetConfiguration(WebContext* svr) {
  char buffer[1000];
  size_t bufferSize = sizeof(buffer);
  int size = bufferSize;
  snprintf_P(buffer,size,ClockDevice_get_config,getDisplayName(),getTimezone());
  svr->send(200, "text/xml", buffer);     
}
