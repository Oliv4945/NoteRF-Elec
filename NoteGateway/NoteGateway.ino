// Sample RFM69 receiver/gateway sketch, with ACK and optional encryption
// Passes through any wireless received messages to the serial port & responds to ACKs
// It also looks for an onboard FLASH chip, if present
// Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/

#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>

#define NODEID        0    //unique for each node on same network
#define NETWORKID     110  //the same on all nodes that talk to each other
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME      30 // max # of ms to wait for an ack
#define SERIAL_BAUD   115200


// Data types
#define DATA_STRING    0
#define DATA_DHT22     1
#define DATA_TELEINFOCLIENT 2




// Structures
struct TeleInfoClientData {
  unsigned long indexHC;
  unsigned long indexHP;
  unsigned int iinst;
  unsigned int papp;
};
struct dht22Data {
  int16_t temperature;
  uint16_t humidity;
};

RFM69 radio;
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);
  Serial.print( "Start" );
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower( true ); //only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(promiscuousMode);
  //radio.setFrequency(919000000);
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
}

byte ackCount=0;
uint32_t packetCount = 0;
void loop() {
  
  char bufferRFM[ 62 ];
  char bufferSerial[ 100 ];
  char* bufferPointer;
  byte i;
  struct dht22Data dht22Data;
  uint16_t sensorBattery;
  
  while (1) {

    if ( radio.receiveDone() ) {
      // Start a new frame for Node-Red
      Serial.print( "\n" );
      for ( i = 0; i < radio.DATALEN; i++ ) {
        bufferRFM[i] =  (char) radio.DATA[i]; // TBC - memcpy, direct ?
      }
      bufferRFM[ i ] = '\0';
      
      
      /*
      // Téléinfo - TBC
      struct TeleInfoClientData ticData;
      memcpy( &ticData, buffer, sizeof( struct TeleInfoClientData ) );
      sprintf( buffer, "%d|%lu|%lu|%u|%u|%d|%d", radio.SENDERID, ticData.indexHP, ticData.indexHC, ticData.iinst, ticData.papp, battery, radio.RSSI );
      */
      
      if ( bufferRFM[0] == DATA_STRING ) {
        Serial.print( '[' ); Serial.print( radio.SENDERID, DEC ); Serial.print( "] " );
        bufferPointer = bufferRFM + 1;
        Serial.print( bufferPointer );
        Serial.print( '\n' );
      }
      if ( bufferRFM[0] == DATA_DHT22 ) {
        bufferPointer = bufferRFM + 1;
        memcpy( &dht22Data, bufferPointer, sizeof( struct dht22Data) );
        bufferPointer += sizeof( struct dht22Data);
        memcpy( &sensorBattery, bufferPointer, sizeof( uint16_t ) );
        sprintf( bufferSerial, "%d|%d|%d|%d|%d\n", radio.SENDERID, dht22Data.temperature, dht22Data.humidity, sensorBattery, radio.RSSI );
        Serial.print( bufferSerial );
      }
      
      
      
      

      Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
      if (promiscuousMode)
      {
        Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
      }
      Serial.print( " - i : " );
      Serial.print( i );
      if (radio.ACKRequested())
      {
        byte theNodeID = radio.SENDERID;
        radio.sendACK();
        Serial.print(" - ACK sent.");
  
        // When a node requests an ACK, respond to the ACK
        // and also send a packet requesting an ACK (every 3rd one only)
        // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
       /* if (ackCount++%3==0)
        {
          Serial.print(" Pinging node ");
          Serial.print(theNodeID);
          Serial.print(" - ACK...");
          delay(3); //need this when sending right after reception .. ?
          if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
            Serial.print("ok!");
          else Serial.print("nothing");
        }*/
      }
      Serial.print( "\n" );
    }
  }
}

