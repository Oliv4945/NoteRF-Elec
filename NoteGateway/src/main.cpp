// Sample RFM69 receiver/gateway sketch, with ACK and optional encryption
// Passes through any wireless received messages to the serial port & responds to ACKs
// It also looks for an onboard FLASH chip, if present
// Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/
#include <Arduino.h>
#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include "configuration.h"




// Data types
#define DATA_STRING    0
#define DATA_DHT22     1
#define DATA_TELEINFOCLIENT 2
#define DATA_DS18B20   3
#define DATA_MOISTURE_SENSOR 4
#define DATA_BMP 5
#define DATA_INTERRUPT 6
#define DATA_VL53L0X 7




// Structures
struct DHT22Data {
  int16_t temperature;
  uint16_t humidity;
};
struct DS18B20Data {
  int16_t temperature;
  uint16_t humidity;
};
struct TeleInfoClientData {
  unsigned long indexHC;
  unsigned long indexHP;
  unsigned int iinst;
  unsigned int papp;
};
struct BMPData {
  int16_t temperature;
  uint32_t pressure;
};

RFM69 radio;
bool promiscuousMode = false; // Set to 'true' to sniff all packets on the same network

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
  sprintf(buff, "\nListening at %d Mhz, NetworkID : %d, GatewayID : %d", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915, NETWORKID, NODEID);
  Serial.println(buff);
}

byte ackCount=0;
uint32_t packetCount = 0;
void loop() {

  char bufferRFM[ 62 ];
  char bufferSerial[ 100 ];
  char* bufferPointer;
  byte i;
  struct DHT22Data dht22Data;
  struct TeleInfoClientData ticData;
  uint8_t dataMoisture;
  uint16_t sensorBattery;
  struct BMPData bmpData;
  uint8_t interruptData;
  uint16_t vl53l0xData;


  while (1) {

    if ( radio.receiveDone() ) {
      // Start a new frame for Node-Red
      Serial.print( "\n" );
      for ( i = 0; i < radio.DATALEN; i++ ) {
        bufferRFM[i] =  (char) radio.DATA[i]; // TBC - memcpy, direct ?
      }
      bufferRFM[ i ] = '\0';



      // Select the message ID - TBC - Switch( bufferRFM[0] );
      if ( bufferRFM[0] == DATA_STRING ) {
        Serial.print( '[' ); 
        Serial.print( radio.SENDERID, DEC ); 
        Serial.print( "] " );
        bufferPointer = bufferRFM + 1;
        Serial.print( bufferPointer );
        Serial.print( '\n' );
      }
      if ( bufferRFM[0] == DATA_DHT22 ) {
        bufferPointer = bufferRFM + 1;
        memcpy( &dht22Data, bufferPointer, sizeof( struct DHT22Data) );
        bufferPointer += sizeof( struct DHT22Data);
        memcpy( &sensorBattery, bufferPointer, sizeof( uint16_t ) );
        sprintf( bufferSerial, "%d|%d|%d|%d|%d|%d\n", radio.SENDERID, DATA_DHT22, dht22Data.temperature, dht22Data.humidity, sensorBattery, radio.RSSI );
        Serial.print( bufferSerial );
      }
      if ( bufferRFM[0] == DATA_TELEINFOCLIENT ) {
        bufferPointer = bufferRFM + 1;
        memcpy( &ticData, bufferPointer, sizeof( struct TeleInfoClientData ) );
        bufferPointer += sizeof( struct TeleInfoClientData);
        memcpy( &sensorBattery, bufferPointer, sizeof( uint16_t ) );
        sprintf( bufferSerial, "%d|%d|%lu|%lu|%u|%u|%d|%d\n", radio.SENDERID, DATA_TELEINFOCLIENT, ticData.indexHP, ticData.indexHC, ticData.iinst, ticData.papp, sensorBattery, radio.RSSI );
        Serial.print( bufferSerial );
      }
      if ( bufferRFM[0] == DATA_MOISTURE_SENSOR ) {
        bufferPointer = bufferRFM + 1;
        memcpy( &dataMoisture, bufferPointer, sizeof( uint8_t ) );
        bufferPointer += sizeof( uint8_t );
        memcpy( &sensorBattery, bufferPointer, sizeof( uint16_t ) );
        sprintf( bufferSerial, "%d|%d|%u|%d|%d\n", radio.SENDERID, DATA_MOISTURE_SENSOR, dataMoisture, sensorBattery, radio.RSSI );
        Serial.print( bufferSerial );
      }
      if ( bufferRFM[0] == DATA_DS18B20 ) {
        bufferPointer = bufferRFM + 1;
        memcpy( &dataMoisture, bufferPointer, sizeof( uint16_t ) );
        bufferPointer += sizeof( uint16_t );
        memcpy( &sensorBattery, bufferPointer, sizeof( uint16_t ) );
        sprintf( bufferSerial, "%d|%d|%d|%d|%d\n", radio.SENDERID, DATA_DS18B20, dataMoisture, sensorBattery, radio.RSSI );
        Serial.print( bufferSerial );
      }
      if ( bufferRFM[0] == DATA_BMP ) {
        bufferPointer = bufferRFM + 1;
        memcpy( &bmpData, bufferPointer, sizeof( struct BMPData ) );
        bufferPointer += sizeof( BMPData );
        memcpy( &sensorBattery, bufferPointer, sizeof( uint16_t ) );
        sprintf( bufferSerial, "%d|%d|%d|%lu|%d|%d\n", radio.SENDERID, DATA_BMP, bmpData.temperature, bmpData.pressure, sensorBattery, radio.RSSI );
        Serial.print( bufferSerial );
      }
      if ( bufferRFM[0] == DATA_INTERRUPT ) {
        bufferPointer = bufferRFM + 1;
        memcpy( &interruptData, bufferPointer, sizeof( uint8_t ) );
        bufferPointer += sizeof( uint8_t );
        memcpy( &sensorBattery, bufferPointer, sizeof( uint16_t ) );
        sprintf( bufferSerial, "%d|%d|%d|%d|%d\n", radio.SENDERID, DATA_INTERRUPT, interruptData, sensorBattery, radio.RSSI );
        Serial.print( bufferSerial );
      }
      if ( bufferRFM[0] == DATA_VL53L0X ) {
        bufferPointer = bufferRFM + 1;
        memcpy( &vl53l0xData, bufferPointer, sizeof( uint16_t ) );
        bufferPointer += sizeof( uint16_t );
        memcpy( &sensorBattery, bufferPointer, sizeof( uint16_t ) );
        sprintf( bufferSerial, "%d|%d|%d|%d|%d\n", radio.SENDERID, DATA_VL53L0X, vl53l0xData, sensorBattery, radio.RSSI );
        Serial.print( bufferSerial );
      }


      Serial.print('[');
      Serial.print(radio.SENDERID, DEC);
      Serial.print("] ");
      if (promiscuousMode)
      {
        Serial.print("to [");
        Serial.print(radio.TARGETID, DEC);
        Serial.print("] ");
      }
      Serial.print( " - radio.DATALEN : " );
      Serial.print( radio.DATALEN );
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
