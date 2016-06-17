/*
  LoRaAT.h - Arduino library for controlling LoRa(WAN?) modules using AT commands.
  
  This library has specifically created to control the MDOT [TODO: Exactly what it is] which is running the
  Australian comptable, AT command enabled [TODO: Can I be more specific?] firmware
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
	void begin();			//Use default baud
	void begin(uint32_t);	//Use specificed baud rate. TODO: Think about other things, e.g. datarate, adaptive data rate.
    int join();				//Join a LoRa(WAN?) network TODO: Think about network parameters, saying something about network?
	int join(unsigned int); //Join with a specifc timeout
    void leave();			//Leave a LoRa(WAN?) network, do we event want this?
	int send(String);		//In general we send strings using the AT command, we could have overloaded functions to accept other things? Maybe data in a particular format?
	int send(String, unsigned int); //Use specific timout like join function.
	int sendPairs(String);
	uint8_t ping();			//I believe there maybe a ping function... I'm not sure yet what the AT command returns, or what we should return to the user?
	String debug;
  private:
	static const uint16_t _jsonMemeoryPool = 500;  //Preallocated memeory for JSON lib in bytes

	static const uint8_t _MAX_FRAGMENTS = 16;
	static const uint8_t _PACKET_SIZE = 11;
	static const uint8_t _HEADER_SIZE = 2;
	static const uint8_t _PAYLOAD_SIZE = _PACKET_SIZE - _HEADER_SIZE;

	String _txBuffer[_MAX_FRAGMENTS];
	uint8_t _txPutter = 0;
	uint8_t _txGetter = 0;
	
    int _findText(String, String);
	String _pairsToJSON(String);
	void _createFragmentBuffer(String);
	int _processBuffer();
	
    uint8_t _u8SerialPort;	//< serial port initialized in constructor
	String _sendQueue[_MAX_FRAGMENTS];
};

#endif