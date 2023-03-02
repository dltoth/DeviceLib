# DeviceLib
DeviceLib is a library of turn-key [UPnPDevices](https://github.com/dltoth/UPnPDevice) based on the [UPnP Device Architecture](http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf). Use of this library requires the companion libraries [CommonUtil](https://github.com/dltoth/CommonUtil) for user interface, [UPnPDevice](https://github.com/dltoth/UPnPDevice) for device architecture, [ssdp](https://github.com/dltoth/ssdp) for service discovery, and [WiFiPortal](https://github.com/dltoth/WiFiPortal) for access point configuration. It includes the following classes:

|Class|Description|
|---|---|
|[ExtendedDevice](https://github.com/dltoth/DeviceLib/blob/main/src/ExtendedDevice.h) |An extended RootDevice that includes ssdp search to find nearby devices and configuration that includes access point reset.
|[HubDevice](https://github.com/dltoth/DeviceLib/blob/main/src/HubDevice.h) |An ExtendedDevice that displays all RootDevices on a local network. With HubDevice, only one mDNS name is necessary, all other devices can be accessed via the HubDevice portal interface.
|[SoftwareClock](https://github.com/dltoth/DeviceLib/blob/main/src/SoftwareClock.h) | A configurable [Sensor](https://github.com/dltoth/UPnPDevice/blob/main/src/SensorDevice.h) that synchronizes system time with an NTP server and provides date and time|
|[Thermometer](https://github.com/dltoth/DeviceLib/blob/main/src/Thermometer.h)|A DHT22 Temperature/Humidity [Sensor](https://github.com/dltoth/UPnPDevice/blob/main/src/SensorDevice.h)|
|[RelayControl](https://github.com/dltoth/DeviceLib/blob/main/src/RelayControl.h)|A [Control](https://github.com/dltoth/UPnPDevice/blob/main/src/Control.h) for managing a relay, with two states ON and OFF|
|[SensorControlledRelay](https://github.com/dltoth/DeviceLib/blob/main/src/SensorControlledRelay.h)|A virtual subclass of RelayControl that allows control of a Relay with a Sensor|
|[HumidityFan](https://github.com/dltoth/DeviceLib/blob/main/src/HumidityFan.h)|A SensorControlledRelay that couples a Thermometer with a RelayControl, with ON/OFF determined by relative humidity|
|[OutletTimer](https://github.com/dltoth/DeviceLib/blob/main/src/OutletTimer.h)|A SensorControlledRelay that couples a SoftwareClock with a RelayControl with ON/OFF determined by time intervals|
|[Hydrometer](https://github.com/dltoth/DeviceLib/blob/main/src/Hydrometer.h)|A soil moisture sensor|
|[ControlServices](https://github.com/dltoth/DeviceLib/blob/main/src/ControlServices.h)|UPnPServices for managing ControlState (ON/OFF) and ControlMode (AUTOMATIC/MANUAL)|
