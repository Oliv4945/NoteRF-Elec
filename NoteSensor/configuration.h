#ifndef CONFIGURATION_H
  #define CONFIGURATION_H
  
  
  /*************          RF configuration      *************/
  #define RFM_NODEID        1      // Unique for each node on same network
  #define RFM_NETWORKID     110    // The same on all nodes that talk to each other
  #define RFM_GATEWAYID     0      // Gateway to reach
  #define RFM_FREQUENCY     RF69_433MHZ
  #define RFM_ENCRYPTKEY    "sampleEncryptKey" // Exactly the same 16 characters/bytes on all nodes!
  #define RFM_IS_RFM69HW           // Uncomment only for RFM69HW! Leave out if you have RFM69W!
  #define RFM_ACK_TIME      30     // max # of ms to wait for an ack
  #define RFM_POWER_LEVEL   10     // Set power level - TBC for RFM96HW [0-32]
  
  /*************          Node configuration      *************/
  // #define BATTERY_VCC      // Sense Arduino's VCC battery with 1.1V internal ref
  #define BATTERY_VIN         // Sense battery with divider on A7
  
  // Data types  -  Uncomment DATA_XXX to activate it
  // #define DATA_DHT22      1 
  #define PIN_DHT22_DATA  4
  #define PIN_DHT22_POWER 5
  
  // #define DATA_TELEINFOCLIENT 2
  #define PIN_TELEINFOCLIENT 3
  
  // #define DATA_DS18B20   3
  #define PIN_DS18B20    6
  
  // #define DATA_MOISTURE_SENSOR 4
  #define PIN_MOISTURE_DATA A1
  #define PIN_MOISTURE_POWER A0
  
  // Others
  // #define DEBUG_ON              // Uncomment to activate debug
  #define SERIAL_BAUD   115200  // Set serial communication speed
  

#endif
