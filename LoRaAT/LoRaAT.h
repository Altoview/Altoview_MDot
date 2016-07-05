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

/*
  Class definition
*/
class LoRaAT
{
  public:
    LoRaAT();				//Use default serial port
	LoRaAT(uint8_t);		//Use specified serial port. TODO: Think about other things we want to define at this point
	LoRaAT(uint8_t,Stream*);//Use a debugging stream.
	void begin();			//Use default baud
	void begin(uint32_t);	//Use specified baud rate. TODO: Think about other things, e.g. datarate, adaptive data rate.
	
	int join();				//Join a LoRa(WAN?) network TODO: Think about network parameters, saying something about network?
	int join(unsigned int); //Join with a specific timeout
    void leave();			//Leave a LoRa(WAN?) network, Not yet implemented
	
	int send(char*);		//In general we send strings using the AT command, we could have overloaded functions to accept other things? Maybe data in a particular format?
	int send(char*, unsigned int); //Use specific timeout like join function.
	int sendPairs(char*);	//Takes key,value pairs, forms a message, and sends to the LoRa server.
    int sendPairs(String); 	//Takes key,value pairs, forms a message, and sends to the LoRa server.
	uint8_t ping();			//Not yet implemented
	
	//Getter and setter functions for mDot Settings.
	//NOT FULLY TESTED AND IMPLEMENTED
	int setFrequencySubBand(char);
    int getFrequencySubBand();
    int setPublicNetwork(char);
    int getPublicNetwork();
    int setNetworkID(char*);		//Also referred to as the AppEUI
    int getNetworkID();				//Also referred to as the AppEUI
    int setNetworkKey(char*);		//Also referred to as the AppKey
    int getNetworkKey();			//Also referred to as the AppKey
    int setDataRate();
    int getDataRate();
    int setRXOutput();
    int getRXOutput();
    int commitSettings();			//Not yet implemented
  private:
	static const uint8_t _MAX_FRAGMENTS = 16;
	static const uint8_t _PACKET_SIZE = 11;
	static const uint8_t _HEADER_SIZE = 2;
	static const uint8_t _PAYLOAD_SIZE = _PACKET_SIZE - _HEADER_SIZE;
	static const uint8_t _MAX_PAIRS_SIZE = 100;
	static const uint8_t _MAX_MDOT_RESPONSE = 120;			//Max number of bytes the mdot might return
	static const uint8_t _MAX_MDOT_COMMAND = 120;			//TODO: Check against the manual for mDot
	char _txBuffer[_MAX_FRAGMENTS][_PACKET_SIZE];
	uint8_t _txPutter = 0;
	uint8_t _txGetter = 0;
	
	void _sendCommand(char*, char*, uint16_t);
	void _pairsToJSON(char*, char*);
	void _createFragmentBuffer(char*);
	int _processBuffer();
	
    uint8_t _u8SerialPort;	//Debugging serial port initialized in constructor
	Stream* _debugStream;
};

#endif