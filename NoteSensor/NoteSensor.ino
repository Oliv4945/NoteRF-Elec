

// From LowPowerLab's node example code

// Includes
#include <RFM69.h>    // Get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <OneWire.h>
#include <LowPower.h>
#include <dht.h>

// Select debug
#define DEBUG_ON  // From low power lab :)
#ifdef DEBUG_ON
  #define DEBUG( input )   { Serial.print(input); delay(1); }
  #define DEBUGDec( input ) { Serial.print( input, DEC); delay(1); }
  #define DEBUGHex( input ) { Serial.println(input, HEX); delay(1); }
  #define DEBUGln( input ) { Serial.println(input); delay(1); }
#else
  #define DEBUG(input);
  #define DEBUGDec( input )
  #define DEBUGHex( input )
  #define DEBUGln(input);
#endif


// Network configuration
#define NODEID        4      // Unique for each node on same network
#define NETWORKID     110    // The same on all nodes that talk to each other
#define GATEWAYID     0
#define FREQUENCY     RF69_433MHZ
#define ENCRYPTKEY    "sampleEncryptKey" // Exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW           // Uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME      30     // max # of ms to wait for an ack
#define RFM_HIGHPOWER true


// Others
#define SERIAL_BAUD   115200

// Node configuration
// #define BATTERY_VCC
#define BATTERY_VIN
// Data types
#define DATA_STRING    0
#define DATA_DHT22     1
// PIN
#define PIN_DHT22_DATA  6
#define PIN_DHT22_POWER 7


// Structures
struct DHT22Data {
  int16_t temperature;
  uint16_t humidity;
};


// Objects
RFM69 radio;
dht DHT22;


void setup() {
  
  // Serial init.
  #ifdef DEBUG_ON
    Serial.begin( SERIAL_BAUD );
  #endif
  
  // RFM69 init.
  DEBUG( "RFM69 initialisation" );
  radio.initialize( FREQUENCY, NODEID, NETWORKID );
#ifdef IS_RFM69HW
  radio.setHighPower( RFM_HIGHPOWER );
#endif
  radio.encrypt( ENCRYPTKEY );
  radio.setPowerLevel( 10 );
  //radio.setFrequency( 919000000 ); //set frequency to some custom frequency - TBC
  char buff[50];
  sprintf( buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915 );
  DEBUGln( buff );
  delay( 10 );
  
  
  
  // Pins
  pinMode( PIN_DHT22_POWER, OUTPUT );
  /*pinMode( PIN_DHT22_DATA, OUTPUT );
  Serial.println( " Power " );
  digitalWrite( PIN_DHT22_POWER, HIGH );
  digitalWrite( PIN_DHT22_DATA, HIGH );
  delay(10000);
  Serial.println( "10s" );
  delay(10000);
  Serial.println( " 20s " );
  delay(10000);
  Serial.println( " 30s " );*/
  
  
  // DHT22 check
  digitalWrite( PIN_DHT22_POWER, 1 );                   // Power the DHT22
  // LowPower.powerDown( SLEEP_4S, ADC_OFF, BOD_OFF ); // Wait DHT22 start
  delay( 4000 );
  int chk = DHT22.read22( PIN_DHT22_DATA );
  switch ( chk ) {
  case DHTLIB_OK:
      DEBUG("DHT22 - OK,\t");
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
  // DISPLAY DATA
  DEBUG( "DHT22 - " );
  DEBUG( DHT22.humidity );
  DEBUG( ",\t" );
  DEBUGln( DHT22.temperature );
  // digitalWrite( PIN_DHT22_POWER, 0);                // Unpower the DHT2 - TBC might be before
  
}


void loop() {
  // Variable initialisation
  char buffer[50];
  boolean requestACK;
  uint8_t bufflen;
  int counter, counterOK, counterNOK, i, time;
  uint16_t vccCentiVolt;
  struct DHT22Data dht22Data;
  uint8_t dataType;
  
  // Variable initialisation
  requestACK = false;
  counter = 0;
  counterOK = 0;
  counterNOK = 0;
  time = 0;
  
  // Send message to the gateway
  bufflen = sizeof( uint8_t );
  dataType = DATA_STRING;
  memcpy( buffer, &dataType, bufflen );
  sprintf( buffer+bufflen, "%s", "Start" );
  bufflen = bufflen + strlen( "Start" );
  
  if ( radio.sendWithRetry( GATEWAYID, &buffer, bufflen ) ) {
    DEBUGln( " ok!" );
    counterOK ++;
  } else {
    DEBUGln( " nothing..." );
    counterNOK ++;
  }
  
  
  while (1) {
    
    // Read VCC level
    #ifdef BATTERY_VCC
      vccCentiVolt = (uint16_t) readVcc();
    #endif
    #ifdef BATTERY_VIN
      vccCentiVolt = (uint16_t) readVin();
    #endif
    DEBUG( "VCC : " ); DEBUG( vccCentiVolt ); DEBUGln( " cV");
  
    // Read DHT22
    // digitalWrite( PIN_DHT22_POWER, 1);                // Power the DHT22
    // LowPower.powerDown( SLEEP_8S, ADC_OFF, BOD_OFF ); // Wait DHT22 start
    DHT22.read22( PIN_DHT22_DATA );                   // TBC - Add control of chk
    dht22Data.temperature = (int16_t) (DHT22.temperature*100.0);
    dht22Data.humidity = (uint16_t) (DHT22.humidity*100.0);
    // digitalWrite( PIN_DHT22_POWER, 0);   // Unpower the DHT22
    

    
    // Prepare buffer
    bufflen = sizeof( uint8_t );
    dataType = DATA_DHT22;
    memcpy( buffer, &dataType, bufflen );
    memcpy( buffer+bufflen, &dht22Data, sizeof( struct DHT22Data ) );
    bufflen = bufflen + sizeof( struct DHT22Data );
    memcpy( buffer+bufflen, &vccCentiVolt, sizeof( uint16_t) );
    bufflen = bufflen + sizeof( uint16_t);
    DEBUG( "Bufflen : " );
    DEBUGln( bufflen );
    
    /*
    struct DHT22Data test;
    memcpy( &test, buffer+1, sizeof( struct DHT22Data ) );
    Serial.print( "Temperature : " );
    Serial.println( test.temperature );
    Serial.print( "Humidité : " );
    Serial.print( test.humidity );
    */

    if ( radio.sendWithRetry( GATEWAYID, &buffer, bufflen ) ) {
      DEBUGln( " ok!" );
      counterOK ++;
    } else {
      DEBUGln( " nothing..." );
      counterNOK ++;
    }
    counter ++;
    radio.sleep();
    
    // Wait 1 min
    while ( time < 7 ) {
      LowPower.powerDown( SLEEP_8S, ADC_OFF, BOD_OFF );
      time++;
      DEBUG( "Time : "); DEBUGln( time );
    }
    LowPower.powerDown( SLEEP_4S, ADC_OFF, BOD_OFF );
    time = 0;
  }
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
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  int result = (high<<8) | low;
 
  result = 109881L / result; // Calculate Vcc (in centiV); 1125300 = 1.1*etalonnage*1023*1000
  return result; // Vcc in millivolts
}

unsigned int readVin() {
  int Vin;
  // Vin = analogRead( A7 )*3.3/1024*2*100*417/406; // Read value*3.3/1024, Times 2 for resistors, times 100 for centivolts, *417/406 for calibration
  Vin = (unsigned int) analogRead( A7 )*0.6619939193;
  return Vin;
  
}
