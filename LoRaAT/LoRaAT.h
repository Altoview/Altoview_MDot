/*
  File: LoRaAT.h

  Version: v0.0.1.1

  Breif: Arduino library for controlling Multitech mDot LoRa modules using 
         AT commands.

  Copyright: This library is published under GNU AGPLv3 license.
             http://choosealicense.com/licenses/agpl-3.0/

  Author: Campbell Scientific Australia Pty Ltd
*/

#ifndef LoRaAT_h
#define LoRaAT_h

#include "Arduino.h"
#include <SoftwareSerial.h>       //Sneaky sofware serial to output debug info

/*
  Class definition
*/
class LoRaAT
{
  public:
    LoRaAT();				//Use default serial port
	LoRaAT(uint8_t);		//Use specified serial port. TODO: Think about other things we want to define at this point
	LoRaAT(uint8_t,Stream*);
	void begin();			//Use default baud
	void begin(uint32_t);	//Use specificed baud rate. TODO: Think about other things, e.g. datarate, adaptive data rate.
    int init();				//Initilise with default settings
	int join();				//Join a LoRa(WAN?) network TODO: Think about network parameters, saying something about network?
	int join(unsigned int); //Join with a specifc timeout
    void leave();			//Leave a LoRa(WAN?) network, do we event want this?
	int send(char*);		//In general we send strings using the AT command, we could have overloaded functions to accept other things? Maybe data in a particular format?
	int send(char*, unsigned int); //Use specific timout like join function.
	int sendPairs(char*);
    int sendPairs(String); 
	uint8_t ping();			//I believe there maybe a ping function... I'm not sure yet what the AT command returns, or what we should return to the user?
	int setFrequencySubBand(char);
    int getFrequencySubBand();
    int setPublicNetwork();
    int getPublicNetwork();
    int setNetworkID();
    int getNetworkID();
    int setNetworkKey();
    int getNetworkKey();
    int setDataRate();
    int getDataRate();
    int setRXO();
    int getRXO();
    int commitSettings();
  private:
	static const uint8_t _MAX_FRAGMENTS = 16;
	static const uint8_t _PACKET_SIZE = 11;
	static const uint8_t _HEADER_SIZE = 2;
	static const uint8_t _PAYLOAD_SIZE = _PACKET_SIZE - _HEADER_SIZE;
	static const uint8_t _MAX_PAIRS_SIZE = 100;
	static const uint8_t _MAX_MDOT_RESPONSE = 100;			//Max number of bytes the mdot might return
	static const uint8_t _MAX_MDOT_COMMAND = 100;
	char _txBuffer[_MAX_FRAGMENTS][_PACKET_SIZE];
	uint8_t _txPutter = 0;
	uint8_t _txGetter = 0;
	
	void _sendCommand(char*, char*, uint16_t);
	void _pairsToJSON(char*, char*);
	void _createFragmentBuffer(char*);
	int _processBuffer();
	
    uint8_t _u8SerialPort;	//< serial port initialized in constructor
	Stream* _debugStream;
};

#endif