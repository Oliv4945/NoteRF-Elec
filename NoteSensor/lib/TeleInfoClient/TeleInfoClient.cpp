#include "SoftwareSerial.h"
#include "TeleInfoClient.h"




TeleInfoClient::TeleInfoClient( int pinRx, int pinTx, int pinSupply ) {
	cptSerial = new SoftwareSerial( pinRx, pinTx );
	cptSerial->begin( 1200 );
	_pinSupply = pinSupply;
	pinMode(pinRx, INPUT_PULLUP);
}
TeleInfoClient::TeleInfoClient( int pinRx, HardwareSerial &print ) {
	_pinSupply = pinRx;
	pinMode(_pinSupply, INPUT_PULLUP);
}

bool TeleInfoClient::getValues() {
	  // Variables declaration
  char charIn, buffer[ 70 ];
  char checkSum;
  int i, completed, checkSumErrors;
  bool checkSum1, checkSum2, checkSum3, checkSum4, checkSum5;
  
  // Variables initialisation
  charIn = 0;
  i = 0;
  completed = 0;
  checkSumErrors = 0;
  checkSum1 = false;
  checkSum2 = false;
  checkSum3 = false;
  checkSum4 = false;
  checkSum5 = false;
  
  cptSerial->begin( 1200 );
  
  Serial.println( "Passe 1" );
  
  
  while( (checkSum1 & checkSum2 & checkSum3 & checkSum4 & checkSum5) == false ) {
    
    if ( cptSerial->available() > 0 ) {
     buffer[i] = cptSerial->read() & 0x7F;
     Serial.print( buffer[i] );
      checkSum = checkSum + buffer[i];
      if ( (buffer[i] == 0x02) || (buffer[i] == 0x0A ) || ( i>=70 ) ) {
        i = 0;
        checkSum = 0;
      } else {
      	
        if ( buffer[i] == 0x0D ) {
          checkSum = checkSum - buffer[i];
          checkSum = checkSum - buffer[i-1];
          checkSum = checkSum - buffer[i-2];
          checkSum = (checkSum & 0x3F) + 0x20;
          if ( checkSum == buffer[i-1] ) {
            buffer[i-2] = 0;
            if ( strncmp( "HCHP", buffer, 4 ) == 0 ) {
              hchp = atol( &buffer[6] );
              checkSum1 = true;     
            } else if ( strncmp( "HCHC", buffer, 4 ) == 0 ) {
              hchc = atol( &buffer[6] );
              checkSum2 = true;
            } else if ( strncmp( "IINST", buffer, 5 ) == 0 ) { 
              iinst = atol( &buffer[7] );
              checkSum3 = true;
            } else if ( strncmp("PAPP", buffer, 4) == 0 ) {
              papp = atol( &buffer[6] ); 
              checkSum4 = true;    
            } else if ( strncmp("PTEC ", buffer, 5 ) == 0 ) {
              strncpy( ptec, &buffer[5], 4 );
              ptec[4] = '\0';
              checkSum5 = true;
            }
          } else {
	        checkSumErrors ++;
	        if (checkSumErrors > 10 ) {
	        	cptSerial->end();
	        	return false;
	        }
	        buffer[i-2] = 0;
          }
        } 
        i++;
      }
    }
    
  }
  cptSerial->end();
  return true;
}

bool TeleInfoClient::getValues2() {
	  // Variables declaration
  char charIn, buffer[ 300 ];
  char checkSum;
  int i, completed, checkSumErrors;
  bool checkSum1, checkSum2, checkSum3, checkSum4, checkSum5;
  
  // Variables initialisation
  charIn = 0;
  i = 0;
  completed = 0;
  checkSumErrors = 0;
  checkSum1 = false;
  checkSum2 = false;
  checkSum3 = false;
  checkSum4 = false;
  checkSum5 = false;
  
  while( (checkSum1 & checkSum2 & checkSum3 & checkSum4 & checkSum5) == false ) {
    
    buffer[i] = read() & 0x7F;
    checkSum = checkSum + buffer[i];
    if ( (buffer[i] == 0x02) || (buffer[i] == 0x0A ) || ( i>=300 ) ) {
      i = 0;
      checkSum = 0;
    } else {
      if ( buffer[i] == 0x0D ) {
        checkSum = checkSum - buffer[i];
        checkSum = checkSum - buffer[i-1];
        checkSum = checkSum - buffer[i-2];
        checkSum = (checkSum & 0x3F) + 0x20;
        if ( checkSum == buffer[i-1] ) {
          buffer[i-2] = 0;
          if ( strncmp( "HCHP", buffer, 4 ) == 0 ) {
            hchp = atol( &buffer[6] );
            checkSum1 = true;     
          } else if ( strncmp( "HCHC", buffer, 4 ) == 0 ) {
            hchc = atol( &buffer[6] );
            checkSum2 = true;
          } else if ( strncmp( "IINST", buffer, 5 ) == 0 ) { 
            iinst = atol( &buffer[7] );
            checkSum3 = true;
          } else if ( strncmp("PAPP", buffer, 4) == 0 ) {
            papp = atol( &buffer[6] ); 
            checkSum4 = true;    
          } else if ( strncmp("PTEC ", buffer, 5 ) == 0 ) {
            strncpy( ptec, &buffer[5], 4 );
            ptec[4] = '\0';
            checkSum5 = true;
          }
        } else {
       checkSumErrors ++;
       if (checkSumErrors > 10 ) {
       	return false;
       }
       buffer[i-2] = 0;
        }
      } 
      i++;
    }
  }
  return true;
}



char TeleInfoClient::read() {
  int _bitPeriod = 1000000 / 1200;
  char val = 0;
  int bitDelay = _bitPeriod - clockCyclesToMicroseconds(50);
  
  // one byte of serial data (LSB first)
  // ...--\    /--\/--\/--\/--\/--\/--\/--\/--\/--...
  //	 \--/\--/\--/\--/\--/\--/\--/\--/\--/
  //	start  0   1   2   3   4   5   6   7 stop

  while (digitalRead(_pinSupply));

  // confirm that this is a real start bit, not line noise
  if (digitalRead(_pinSupply) == LOW) {
    // frame start indicated by a falling edge and low start bit
    // jump to the middle of the low start bit
    delayMicroseconds(bitDelay / 2 - clockCyclesToMicroseconds(50));
	
    // offset of the bit in the byte: from 0 (LSB) to 7 (MSB)
    for (int offset = 0; offset < 8; offset++) {
	// jump to middle of next bit
	delayMicroseconds(bitDelay);
	
	// read bit
	val |= digitalRead(_pinSupply) << offset;
    }
	
    delayMicroseconds(_bitPeriod);
    
    return val&0x7F;
  }
  
  return -1;
}