
#ifndef TELEINFOCLIENT_h
#define TELEINFOCLIENT_h
#include "Arduino.h"            // assumes Arduino IDE v1.0 or greater



class TeleInfoClient {
  	public:
	    long hchp;
	    long hchc;
	    int iinst;
	    int papp;
	    char ptec[5];
	    
	    TeleInfoClient( int pinRx, int pinTx, int pinSupply );
	    TeleInfoClient( int pinRx, HardwareSerial &print );
	    bool getValues();
	    bool getValues2();
    
	private:
		SoftwareSerial* cptSerial;
		int _pinSupply;
		char read();
    
};

#endif
