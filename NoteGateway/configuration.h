#ifndef CONFIGURATION_H
  #define CONFIGURATION_H
  
  /*************          RF configuration      *************/
  #define NODEID        0                  // Unique for each node on same network
  #define NETWORKID     110                // The same on all nodes that talk to each other
  //Match frequency to the hardware version of the radio on your Moteino (uncomment one):
  #define FREQUENCY     RF69_433MHZ         // Frequency : RF69_868MHZ : RF69_915MHZ
  #define ENCRYPTKEY    "sampleEncryptKey"  //exactly the same 16 characters/bytes on all nodes!
  #define IS_RFM69HW                        //uncomment only for RFM69HW! Leave out if you have RFM69W!
  #define ACK_TIME      30                  // max # of ms to wait for an ack
  
  // Others
  #define SERIAL_BAUD   115200

#endif
