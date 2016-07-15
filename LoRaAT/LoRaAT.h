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
    char frequencySubBand = '\0';   //0-8
	char publicNetwork = '\0';		//0,1
	char networkId[24] = {'\0'};    //00:00:aa:00:00:00:00:01
	char networkKey[48] = {'\0'};	//00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01
    char dataRate = '\0';			//0-3
	
	LoRaAT();				        //Use default serial port
	LoRaAT(uint8_t);		        //Use specified serial port.
	LoRaAT(uint8_t,Stream*);        //Use specified serial port and a debugging stream.
	
	void begin();			        //Use default baud
	void begin(uint32_t);	        //Use specified baud rate.
	
	int join();				        //Join a LoRa network.
	int join(unsigned int);         //Join with a specific timeout
    void leave();			        //Leave a LoRa network, Not yet implemented
	
	int send(char*);		        //Generic send command, using AT+SEND
	int send(char*, unsigned int);  //Use specific timeout.
	int sendPairs(char*);	        //Takes key,value pairs, forms a message, and sends to the LoRa server.
    int sendPairs(String); 	        //Takes key,value pairs, forms a message, and sends to the LoRa server.
	uint8_t ping();			        //Not yet implemented
	
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
    int setDataRate(char);
    int getDataRate();
    int setRXOutput(uint8_t);
    int getRXOutput();
    int commitSettings();			//Not yet implemented
  
  private:
	static const uint8_t _MAX_FRAGMENTS = 16;
	static const uint8_t _PACKET_SIZE = 11;
	static const uint8_t _HEADER_SIZE = 2;
	static const uint8_t _PAYLOAD_SIZE = _PACKET_SIZE - _HEADER_SIZE;
	static const uint8_t _HEADER_OFFSET = 48;
    
	static const uint8_t _MAX_PAIRS_SIZE = 100;
    
	char _txBuffer[_MAX_FRAGMENTS][_PACKET_SIZE];
	uint8_t _txPutter = 0;
	uint8_t _txGetter = 0;
	
	static const uint8_t _MAX_MDOT_RESPONSE = 150;			//Max number of bytes the mdot might return
	char _response[_MAX_MDOT_RESPONSE];						//mDot response buffer
	uint8_t _length;										//Lenght of a response
	
	static const uint8_t _MAX_MDOT_COMMAND = 120;			//TODO: Check against the manual for mDot
	char _command[_MAX_MDOT_COMMAND];
	
	uint8_t _sendCommand(char*, char*, char*, char*, char*, uint16_t);
	uint8_t _sendCommand(char*, char*, char*, char*, char*, uint16_t, char*);             //Generic serial out get response wrapper
	
	void _pairsToJSON(char*, char*);
	void _createFragmentBuffer(char*);
	int _processBuffer();
	
    uint8_t _u8SerialPort;                                 //AT Command serial port selection by user
	Stream* _debugStream;                                  //Debugging serial port initialized in constructor
};

#endif