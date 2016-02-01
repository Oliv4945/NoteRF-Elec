# NodeRF

RF sensors and actuators based on ATMega328 and RFM69hw from hopeRF.  
Arduino bootloader is used in order to use all available libraries

## NoteGateway
Gateway with serial output. It just collects data from RF sensors and send it through serial link.  
Configuration is explained in configuration.h file

Serial messages are really simple for now and formated like `sensorNumber|dataType|Data1|Data2|DataX|batteryLevel|RSSI`  
For example, the sensor N°5, type temperature (16.50°C) and humidity (55.3%) with a battery level of 3.98V is `5|1|1650|5530|398|-53`

## NoteSensor
Sensors' code.  
You just have to set config.h to suit your needs. Also uncommenting a `#define DATA_SENSORNAME` will activate it.

## Schematics
Hardware source files made with Eagle 7.3.0 + [Spakfun library](https://github.com/sparkfun/SparkFun-Eagle-Libraries)

