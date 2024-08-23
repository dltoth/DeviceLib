# DeviceLib
DeviceLib is a library of turn-key [UPnPDevices](https://github.com/dltoth/UPnPDevice) based on the [UPnP Device Architecture](http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf) for ESP8266 and ESP32. Use of this library requires the companion libraries [CommonUtil](https://github.com/dltoth/CommonUtil) for user interface, [UPnPDevice](https://github.com/dltoth/UPnPDevice) for device architecture, [ssdp](https://github.com/dltoth/ssdp) for service discovery, and [WiFiPortal](https://github.com/dltoth/WiFiPortal) for access point configuration. 

DeviceLib includes the following classes:

```
   ExtendedDevice         := An extended RootDevice that includes ssdp search to find nearby devices and configuration that includes access point reset
   HubDevice              := An ExtendedDevice that displays all RootDevices on a local network. With HubDevice, only one mDNS name is necessary, all other devices can be 
                             accessed via the HubDevice portal interface
   Sensor                 := A configurable UPnPDevice base class providing Sensor reading as simple HTML and Sensor configuration through ConfigurationServices
   SoftwareClock          := A Sensor that synchronizes system time with an NTP server and provides date and time
   Thermometer            := A DHT22 Temperature/Humidity Sensor. Thermometer requires the additional library DHT_sensor_library_for_ESPx
   Hydrometer             := A Sensor that reads from the analog pin and computes soil moisture content
   Control                := A configurable UPnPDevice base class providing its Control UI thru an iFrame and configuration through ConfigurationServices
   RelayControl           := A Control for managing a relay with two states ON and OFF
   SensorControlledRelay  := A virtual subclass of RelayControl that allows control of a Relay with a Sensor
   HumidityFan            := A SensorControlledRelay that couples a Thermometer with a RelayControl, with ON/OFF determined by relative humidity
   OutletTimer            := A SensorControlledRelay that couples a SoftwareClock with a RelayControl with ON/OFF determined by time intervals
   ControlServices        := UPnPServices for managing ControlState (ON/OFF) and ControlMode (AUTOMATIC/MANUAL)
   ConfigurationServices  := UPnPServices for managing configuration (GetConfiguration/SetConfiguration)
```

The library is centered around the two base classes [Sensor](https://github.com/dltoth/DeviceLib/blob/main/src/SensorDevice.h), a configurable UPnPDevice with simple HTML display, and [Control](https://github.com/dltoth/DeviceLib/blob/main/src/Control.h), a configurable UPnPDevice with complex HTML display. It also includes a turn-key Device Hub, [HubDevice](https://github.com/dltoth/DeviceLib/blob/main/src/HubDevice.h), that can be included in a boilerplate sketch to provide access to all UPnPRootDevices on a local network.

## Basic Usage

To see how this works, consider a [simple sketch](https://github.com/dltoth/DeviceLib/blob/main/examples/RelayControl/RelayControl.ino) consisting of an [ExtendedDevice](https://github.com/dltoth/DeviceLib/blob/main/src/ExtendedDevice.h) with a Control ([RelayControl](https://github.com/dltoth/DeviceLib/blob/main/src/RelayControl.h)), and Sensor ([SoftwareClock](https://github.com/dltoth/DeviceLib/blob/main/src/SoftwareClock.h)). The sketch will not be reviewed in detail but notice device instantiation is as follows:

```
SSDP             ssdp;
ExtendedDevice   root;
SoftwareClock    c;
RelayControl     relay;
const char*      deviceName = "Outlet";
```

SSDP is used for Service discovery, ExtendedDevice is used as the RootDevice container to provide configuration and Device search, and RootDevice display name will be "Outlet". No, flash an ESP device with the sketch above, start it up and point a browser to the device IP address.

*Figure 1 - RelayControl display at http://device-IP:80*

![image1](/assets/image1.png)






