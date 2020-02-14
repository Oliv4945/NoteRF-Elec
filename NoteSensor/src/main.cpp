// Includes
#include <Arduino.h>
#include <RFM69.h>    // Get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <LowPower.h>
#include "configuration.h"
#ifdef DATA_DHT22
  #include <dht.h>
#endif
#ifdef DATA_TELEINFOCLIENT
  #include <SoftwareSerial.h>
  #include <TeleInfoClient.h>
#endif
#ifdef DATA_BMP
  // From https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/BMP085
  #include "Wire.h"
  #include "I2Cdev.h"
  #include "BMP085.h"
#endif
#ifdef DATA_VL53L0X
  #include <Wire.h>
  #include <VL53L0X.h>
#endif

// Activate/desactivate debug with only one #define
// From low power lab :)
#ifdef DEBUG_ON 
  #define DEBUG( input )    { Serial.print(input); delay(1); }
  #define DEBUGDec( input ) { Serial.print( input, DEC); delay(1); }
  #define DEBUGHex( input ) { Serial.print(input, HEX); delay(1); }
  #define DEBUGln( input )  { Serial.println(input); delay(1); }
#else
  #define DEBUG(input);
  #define DEBUGDec( input )
  #define DEBUGHex( input )
  #define DEBUGln(input);
#endif


// Others - This data has not to be configured
#define DATA_STRING    0


// Structures
#ifdef DATA_DHT22
  struct DHT22Data {
    int16_t temperature;
    uint16_t humidity;
  };
#endif
#ifdef DATA_TELEINFOCLIENT
  struct TeleInfoClientData {
    unsigned long indexHC;
    unsigned long indexHP;
    unsigned int iinst;
    unsigned int papp;
  };
#endif
#ifdef DATA_DS18B20
  struct DS18B20Data {
    int16_t temperature;
    uint16_t humidity;
  };
#endif
#ifdef DATA_BMP
  struct BMPData {
    int16_t temperature;
    uint32_t pressure;
  };
#endif

// Objects and globals
RFM69 radio;
#ifdef DATA_DHT22
  dht DHT22;
#endif
#ifdef DATA_TELEINFOCLIENT
  TeleInfoClient tic( 3, 10, 11 );
#endif
#ifdef DATA_BMP
  BMP085 barometer;
#endif
#ifdef DATA_INTERRUPT
  uint8_t interruptData;
#endif

#ifdef DATA_VL53L0X
  VL53L0X tof;
#endif
  

void setup() {
  
  // Serial init.
  #ifdef DEBUG_ON
    Serial.begin( SERIAL_BAUD );
  #endif
  DEBUG( "SensorRF - NodeID : " );
  DEBUGln( RFM_NODEID );
  
  // RFM69 init.
  DEBUG( "RFM69 - Initialisation" );
  radio.initialize( RFM_FREQUENCY, RFM_NODEID, RFM_NETWORKID );
  DEBUG( "RFM69 - Initialised" );
  #ifdef RFM_IS_RFM69HW
    radio.setHighPower( true );
  #endif
  radio.encrypt( RFM_ENCRYPTKEY );
  radio.setPowerLevel( RFM_POWER_LEVEL );
  //radio.setFrequency( 919000000 ); //set frequency to some custom frequency - TBC
  char buff[50];
  sprintf( buff, "\nTransmitting at %d Mhz...", RFM_FREQUENCY==RF69_433MHZ ? 433 : RFM_FREQUENCY==RF69_868MHZ ? 868 : 915 );
  DEBUGln( buff );
  delay( 10 );
  
  
  #ifdef DATA_DHT22
    // Pins
    pinMode( PIN_DHT22_POWER, OUTPUT );
    // DHT22 check
    digitalWrite( PIN_DHT22_POWER, HIGH );                   // Power the DHT22
    LowPower.powerDown( SLEEP_4S, ADC_OFF, BOD_OFF );        // Wait DHT22 start
    int chk = DHT22.read22( PIN_DHT22_DATA );
    switch ( chk ) {
    case DHTLIB_OK:
        DEBUG("DHT22 - OK,\t");
        // DISPLAY DATA
        DEBUG( DHT22.humidity );
        DEBUG( "% ,\t" );
        DEBUG( DHT22.temperature );
        DEBUGln( "C" );
        break;
    case DHTLIB_ERROR_CHECKSUM:
        DEBUG("DHT22 - Checksum error,\t");
        break;
    case DHTLIB_ERROR_TIMEOUT:
        DEBUG("DHT22 - Time out error,\t");
        break;
    case DHTLIB_ERROR_CONNECT:
        DEBUG("DHT22 - Connect error,\t");
        break;
    case DHTLIB_ERROR_ACK_L:
        DEBUG("DHT22 - Ack Low error,\t");
        break;
    case DHTLIB_ERROR_ACK_H:
        DEBUG("DHT22 - Ack High error,\t");
        break;
    default:
        DEBUG("DHT22 - Unknown error,\t");
        break;
    }
    // digitalWrite( PIN_DHT22_POWER, 0);                // Unpower the DHT2 - TBC might be before, check library
    LowPower.powerDown( SLEEP_1S, ADC_OFF, BOD_OFF );    // Avoid error on first reading in loop()
  #endif
  
  #ifdef DATA_MOISTURE_SENSOR
    // Setup the outputs
    pinMode( PIN_MOISTURE_POWER, OUTPUT );
    pinMode( PIN_MOISTURE_DATA, INPUT );
    digitalWrite( PIN_MOISTURE_POWER, OUTPUT );
  #endif
  #ifdef DATA_BMP
    Wire.begin();
    barometer.initialize();
    #ifdef DEBUG_ON
      Serial.print( barometer.testConnection() ? "BMP - Connection successful" : "BMP - Connection failed") ;
    #endif
  #endif
  #ifdef DATA_INTERRUPT
    pinMode(3, INPUT);
  #endif
  #ifdef DATA_VL53L0X
      tof.init();
      tof.setTimeout(VL53L0X_TIMING_REC_us*2/1000);
      tof.setMeasurementTimingBudget(VL53L0X_TIMING_REC_us);

      // Set long range
      // lower the return signal rate limit (default is 0.25 MCPS)
      tof.setSignalRateLimit(0.1);
      // increase laser pulse periods (defaults are 14 and 10 PCLKs)
      tof.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
      tof.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  #endif
}

unsigned int readVin() {
  int Vin;
  // Vin = analogRead( A7 )*3.3/1024*2*100*417/406; // Read value*3.3/1024, Times 2 for resistors, times 100 for centivolts, *417/406 for calibration
  Vin = (unsigned int) analogRead( A7 )*0.6619939193; // TBC - Add calibration value in configuration.h
  return Vin;
  
}
// http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
int readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay( 2 );                       // Wait for Vref to settle
  ADCSRA |= _BV( ADSC );            // Start conversion
  while (bit_is_set(ADCSRA,ADSC));  // Measuring
 
  uint8_t low  = ADCL;              // Must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH;              // Unlocks both
 
  int result = (high<<8) | low;
 
  result = 109881L / result;        // Calculate Vcc (in centiV); 1125300 = 1.1*etalonnage*1023*1000
  return result;                    // Vcc in millivolts
}


// Interruption sub routine on pin 3
#ifdef DATA_INTERRUPT
  void isr_D3() {
    interruptData = 1;
  }
#endif

void loop() {
  // Variable declaration
  char buffer[50];
  boolean requestACK;
  uint8_t bufflen;
  int time;
  uint16_t vccCentiVolt;
  uint8_t dataType;
  int32_t lastMicros;
  
  #ifdef DATA_DHT22
    struct DHT22Data dht22Data;
    int dht22_check;
  #endif
  #ifdef DATA_TELEINFOCLIENT
    struct TeleInfoClientData ticData;
  #endif
  #ifdef DATA_MOISTURE_SENSOR
    uint8_t dataMoisture;
  #endif
  #ifdef DATA_BMP
    struct BMPData bmpData;
  #endif
  
  // Variable initialisation
  requestACK = false;
  time = 0;
  #ifdef DATA_INTERRUPT
    interruptData = 0;
  #endif
  
  // Send message to the gateway
  bufflen = sizeof( uint8_t );
  dataType = DATA_STRING;
  memcpy( buffer, &dataType, bufflen );
  sprintf( buffer+bufflen, "%s", "Start" );
  bufflen = bufflen + strlen( "Start" );
  
  if ( radio.sendWithRetry( RFM_GATEWAYID, &buffer, bufflen ) ) {
    DEBUGln( "RFM - OK" );
  } else {
    DEBUGln( "RFM - NOK" );
  }
  
  
  while (1) {
    // Reset sending buffer length
    bufflen = 0;
    
    // Read VCC level
    #ifdef BATTERY_VCC
      vccCentiVolt = (uint16_t) readVcc();
    #endif
    #ifdef BATTERY_VIN
      vccCentiVolt = (uint16_t) readVin();
    #endif
    DEBUG( "VCC : " ); DEBUG( vccCentiVolt ); DEBUGln( " cV");
  
    #ifdef DATA_DHT22
      // Read DHT22
      // digitalWrite( PIN_DHT22_POWER, 1);                // Power the DHT22
      // LowPower.powerDown( SLEEP_8S, ADC_OFF, BOD_OFF ); // Wait DHT22 start
      dht22_check = DHT22.read22( PIN_DHT22_DATA );   
      if ( dht22_check == DHTLIB_OK ) {    
        dht22Data.temperature = (int16_t) (DHT22.temperature*100.0);
        dht22Data.humidity = (uint16_t) (DHT22.humidity*100.0);
        // digitalWrite( PIN_DHT22_POWER, 0);   // Unpower the DHT22
        
        // Add data to the buffer
        dataType = DATA_DHT22;
        memcpy( buffer+bufflen, &dataType, sizeof( uint8_t ) );
        bufflen = bufflen + sizeof( uint8_t );
        memcpy( buffer+bufflen, &dht22Data, sizeof( struct DHT22Data ) );
        bufflen = bufflen + sizeof( struct DHT22Data );
        // Prepare buffer
        memcpy( buffer+bufflen, &vccCentiVolt, sizeof( uint16_t) );
        bufflen = bufflen + sizeof( uint16_t);
        DEBUG( "Bufflen : " );
        DEBUGln( bufflen );
        
        if ( radio.sendWithRetry( RFM_GATEWAYID, &buffer, bufflen ) ) {
          DEBUGln( "RFM - OK" );
        } else {
          DEBUGln( "RFM - NOK" );
        }
      
      } else {  // TBC - Send error via RF
        DEBUG( "DHT22 - Check NOK : " );
        DEBUGln( dht22_check );
      }
    #endif
    
    // TBC - Avoid sending VCC twice - Separate data ?
    
    #ifdef DATA_TELEINFOCLIENT // TBC - Add timeout
      tic.getValues();
      ticData.indexHP = tic.hchp;
      ticData.indexHC = tic.hchc;
      ticData.iinst = tic.iinst;
      ticData.papp = tic.papp;
      DEBUG( "PAPP - ");
      DEBUGln( tic.papp );
      
      
      // Add data to the buffer
      bufflen = 0;
      dataType = DATA_TELEINFOCLIENT;
      memcpy( buffer+bufflen, &dataType, sizeof( uint8_t ) );
      bufflen = bufflen + sizeof( uint8_t );
      memcpy( buffer+bufflen, &ticData, sizeof( struct TeleInfoClientData ) );
      bufflen = bufflen + sizeof( struct TeleInfoClientData );
      memcpy( buffer+bufflen, &vccCentiVolt, sizeof( uint16_t) );
      bufflen = bufflen + sizeof( uint16_t);
      if ( radio.sendWithRetry( RFM_GATEWAYID, &buffer, bufflen ) ) {
        DEBUGln( "RFM - OK" );
      } else {
        DEBUGln( "RFM - NOK" );
      }
    #endif

    /*****      MOISTURE SENSOR     ******/
    #ifdef DATA_MOISTURE_SENSOR
      // Power up the moisture sensor
      digitalWrite( PIN_MOISTURE_POWER, HIGH );
      // Wait some time for the current to settle - time TBC
      delay( 100 );
      // Read data and divide it by 4 to fit inside 1 byte, enough for the required precision
      dataMoisture = analogRead( PIN_MOISTURE_DATA )/4;
      // Power down the sensor
      digitalWrite( PIN_MOISTURE_POWER, LOW );
      DEBUG( "MOISTURE - Value : " );
      DEBUGln( dataMoisture );
      
      // Add data to the buffer
      bufflen = 0;
      dataType = DATA_MOISTURE_SENSOR;
      memcpy( buffer+bufflen, &dataType, sizeof( uint8_t ) );
      bufflen = bufflen + sizeof( uint8_t );
      memcpy( buffer+bufflen, &dataMoisture, sizeof( uint8_t ) );
      bufflen = bufflen + sizeof( uint8_t );
      memcpy( buffer+bufflen, &vccCentiVolt, sizeof( uint16_t) );
      bufflen = bufflen + sizeof( uint16_t);
      if ( radio.sendWithRetry( RFM_GATEWAYID, &buffer, bufflen ) ) {
        DEBUGln( "RFM - OK" );
      } else {
        DEBUGln( "RFM - NOK" );
      }
    #endif


    /*****      BAROMETRIC SENSOR     ******/
    #ifdef DATA_BMP
      // Request temperature
      barometer.setControl( BMP085_MODE_TEMPERATURE );
      // Wait appropriate time for conversion (4.5ms delay)
      lastMicros = micros();
      while (micros() - lastMicros < barometer.getMeasureDelayMicroseconds());
      // Read calibrated temperature value in degrees Celsius
      bmpData.temperature = ( (int16_t) (barometer.getTemperatureC()*100.0) );
      // Request pressure (3x oversampling mode, high detail, 23.5ms delay)
      barometer.setControl( BMP085_MODE_PRESSURE_3 );
      while (micros() - lastMicros < barometer.getMeasureDelayMicroseconds());
      // Read calibrated pressure value in Pascals (Pa)
      bmpData.pressure = ( (uint32_t) barometer.getPressure() );
      // Display values
      DEBUG( "BMP - Temperature : " );
      DEBUGln( bmpData.temperature );
      DEBUG( "BMP - Pressure : " );
      DEBUGln( bmpData.pressure );
      
      // Add data to the buffer
      bufflen = 0;
      dataType = DATA_BMP;
      memcpy( buffer+bufflen, &dataType, sizeof( uint8_t ) );
      bufflen = bufflen + sizeof( uint8_t );
      memcpy( buffer+bufflen, &bmpData, sizeof( struct BMPData ) );
      bufflen = bufflen + sizeof( struct BMPData );
      // Prepare buffer
      memcpy( buffer+bufflen, &vccCentiVolt, sizeof( uint16_t) );
      bufflen = bufflen + sizeof( uint16_t);
      DEBUG( "Bufflen : " );
      DEBUGln( bufflen );
      if ( radio.sendWithRetry( RFM_GATEWAYID, &buffer, bufflen ) ) {
        DEBUGln( "RFM - OK" );
      } else {
        DEBUGln( "RFM - NOK" );
      }
    #endif


    #ifdef DATA_INTERRUPT
      // Send interrupt on wake up
      if (interruptData != 0) {
        DEBUG( "INTERRUPT - Triggered : " );
        DEBUGln( interruptData );
        
        // Add data to the buffer
        bufflen = 0;
        dataType = DATA_INTERRUPT;
        memcpy( buffer+bufflen, &dataType, sizeof( uint8_t ) );
        bufflen = bufflen + sizeof( uint8_t );
        memcpy( buffer+bufflen, &interruptData, sizeof( uint8_t ) );
        bufflen = bufflen + sizeof(  uint8_t );
        // Prepare buffer
        memcpy( buffer+bufflen, &vccCentiVolt, sizeof( uint16_t) );
        bufflen = bufflen + sizeof( uint16_t);
        DEBUG( "Bufflen : " );
        DEBUGln( bufflen );
        if ( radio.sendWithRetry( RFM_GATEWAYID, &buffer, bufflen ) ) {
          DEBUGln( "RFM - OK" );
        } else {
          DEBUGln( "RFM - NOK" );
        }
        
        interruptData = 0;
        
      }
    #endif

    #ifdef DATA_VL53L0X
      uint16_t distance = tof.readRangeSingleMillimeters();
      if (tof.timeoutOccurred()) {
        DEBUGln("TOF - Timeout");
      } else {
        DEBUG("TOF - Distance: ");
        DEBUG(distance);
        DEBUGln(" mm");
      }
      // Transmit even in case of timeout (0xFFFF)
      
      // Add data to the buffer
      bufflen = 0;
      dataType = DATA_VL53L0X;
      memcpy( buffer+bufflen, &dataType, sizeof( uint8_t ) );
      bufflen = bufflen + sizeof( uint8_t );
      memcpy( buffer+bufflen, &distance, sizeof( uint16_t ) );
      bufflen = bufflen + sizeof( uint16_t );
      // Prepare buffer
      memcpy( buffer+bufflen, &vccCentiVolt, sizeof( uint16_t) );
      bufflen = bufflen + sizeof( uint16_t);
      DEBUG( "Bufflen : " );
      DEBUGln( bufflen );
      if ( radio.sendWithRetry( RFM_GATEWAYID, &buffer, bufflen ) ) {
        DEBUGln( "RFM - OK" );
      } else {
        DEBUGln( "RFM - NOK" );
      }
    #endif
    
    
    // Put radio in sleep mode
    #ifndef DATA_INTERRUPT
        radio.sleep();
      // Wait 1 min minus 2 secondes - TBC for a sending every minute
      while ( time < 7 ) {
        LowPower.powerDown( SLEEP_8S, ADC_OFF, BOD_OFF );
        time++;
        DEBUG( "Time : "); DEBUGln( time );
      }
      LowPower.powerDown( SLEEP_2S, ADC_OFF, BOD_OFF );
      time = 0;
    #endif
    #ifdef DATA_INTERRUPT
      attachInterrupt(digitalPinToInterrupt(3), isr_D3, RISING);
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
      // Disable external pin interrupt on wake up pin.
      detachInterrupt(0);
    #endif
  }
}



