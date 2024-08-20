# DeviceLib
DeviceLib is a library of turn-key [UPnPDevices](https://github.com/dltoth/UPnPDevice) for ESP8266 and ESP32 based on the [UPnP Device Architecture](http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf). Use of this library requires the companion libraries [CommonUtil](https://github.com/dltoth/CommonUtil) for user interface, [UPnPDevice](https://github.com/dltoth/UPnPLib) for device architecture and service discovery, [SystemClock](https://github.com/dltoth/SystemClock) for NTP synchronized time, and [WiFiPortal](https://github.com/dltoth/WiFiPortal) for access point configuration. It includes the following classes:
&nbsp;

&nbsp;

|Class|Description|
|---|---|
|[ExtendedDevice](https://github.com/dltoth/DeviceLib/blob/main/src/ExtendedDevice.h) |An extended RootDevice that includes ssdp search to find nearby devices and configuration that includes access point reset.
|[HubDevice](https://github.com/dltoth/DeviceLib/blob/main/src/HubDevice.h) |An ExtendedDevice that displays all RootDevices on a local network. With HubDevice, only one mDNS name is necessary, all other devices can be accessed via the HubDevice portal interface. HubDevice can be included in a [boilerplate sketch](https://github.com/dltoth/DeviceLib/blob/main/examples/HubDevice/HubDevice.ino) with a SoftwareClock to create an ESP device for the sole purpose of discovery.
|[SoftwareClock](https://github.com/dltoth/DeviceLib/blob/main/src/SoftwareClock.h) | A configurable [Sensor](https://github.com/dltoth/UPnPDevice/blob/main/src/SensorDevice.h) that synchronizes system time with an NTP server and provides date and time|
|[Thermometer](https://github.com/dltoth/DeviceLib/blob/main/src/Thermometer.h)|A DHT22 Temperature/Humidity [Sensor](https://github.com/dltoth/UPnPDevice/blob/main/src/SensorDevice.h). Thermometer requires the additional library DHT_sensor_library_for_ESPx.|
|[RelayControl](https://github.com/dltoth/DeviceLib/blob/main/src/RelayControl.h)|A [Control](https://github.com/dltoth/UPnPDevice/blob/main/src/Control.h) for managing a relay, with two states ON and OFF|
|[SensorControlledRelay](https://github.com/dltoth/DeviceLib/blob/main/src/SensorControlledRelay.h)|A virtual subclass of RelayControl that allows control of a Relay with a Sensor|
|[HumidityFan](https://github.com/dltoth/DeviceLib/blob/main/src/HumidityFan.h)|A SensorControlledRelay that couples a Thermometer with a RelayControl, with ON/OFF determined by relative humidity|
|[OutletTimer](https://github.com/dltoth/DeviceLib/blob/main/src/OutletTimer.h)|A SensorControlledRelay that couples a SoftwareClock with a RelayControl with ON/OFF determined by time intervals|
|[Hydrometer](https://github.com/dltoth/DeviceLib/blob/main/src/Hydrometer.h)|A Sensor that reads from the analog pin and computes soil moisture content.|
|[ControlServices](https://github.com/dltoth/DeviceLib/blob/main/src/ControlServices.h)|UPnPServices for managing ControlState (ON/OFF) and ControlMode (AUTOMATIC/MANUAL)|

The turn-key devices provided by this library can be mix and matched in a [boilerplate sketch](https://github.com/dltoth/DeviceLib/blob/main/examples/DeviceLib/DeviceLib.ino) to create a variaty of ESP devices, discoverable on a local network.
