/*
  File: LoRaAT.cpp

  Version: v0.0.1.1

  Breif: Arduino library for controlling Multitech mDot LoRa modules using 
         AT commands.

  Copyright: This library is published under GNU AGPLv3 license.
             http://choosealicense.com/licenses/agpl-3.0/

  Author: Campbell Scientific Australia Pty Ltd
*/

/************************************************************************************
 *                                   INCLUDES                                       *
 ***********************************************************************************/
#include "Arduino.h"
#include "LoRaAT.h"

/************************************************************************************
 *                               GLOBAL VARIABLES                                   *
 ***********************************************************************************/
#if defined(ARDUINO_ARCH_AVR)
  HardwareSerial* ATSerial = &Serial; //< Pointer to Serial class object
#elif defined(ARDUINO_ARCH_SAM)
  UARTClass* ATSerial = &Serial;      //< Pointer to Serial class object
#else
  #error "This library only supports boards with an AVR or SAM processor."
#endif


/************************************************************************************
 *                               PUBLIC FUNCTIONS                                   *
 ***********************************************************************************/

/*----------------------------------------------------------------------------------|
| CONSTRUCTOR: Creates class object using a default serial port 0					|
-----------------------------------------------------------------------------------*/
 LoRaAT::LoRaAT() {
  _u8SerialPort = 0;
  _debugStream = NULL;
}

/*----------------------------------------------------------------------------------|
| CONSTRUCTOR: Creates class object using a specified serial port.					|
-----------------------------------------------------------------------------------*/
 LoRaAT::LoRaAT(uint8_t u8SerialPort) {
  //TODO: Input checking, what range of values to accept, how to handle invalid input
  _u8SerialPort = u8SerialPort;
  _debugStream = NULL;
}

/*----------------------------------------------------------------------------------|
| CONSTRUCTOR: Creates class object using a specified serial port, and passing it a |
| specificed debug stream. This stream can be used to pass debug info to a serial	|
| port, either hardware serial or software serial.									|
-----------------------------------------------------------------------------------*/
LoRaAT::LoRaAT(uint8_t u8SerialPort, Stream* debugStream) {
  //TODO: Input checking, what range of values to accept, how to handle invalid input
  _u8SerialPort = u8SerialPort;
  _debugStream = debugStream;
}
  
/*----------------------------------------------------------------------------------|
| Initialize class object.															|
| 																					|
| Sets up the serial port using default 19200 baud rate.							|
| Call once class has been instantiated, typically within setup().					|
-----------------------------------------------------------------------------------*/
void LoRaAT::begin(void) {
  //TODO: Input checking??
  begin(19200);
}

/*----------------------------------------------------------------------------------|
| Initialize class object.															|
| 																					|
| Sets up the serial port using default 19200 baud rate.							|
| Call once class has been instantiated, typically within setup().					|
|																					|
| TODO: Think about what we should set here, what should be defaults. Datarates,	|
| adaptive data rates? Other things?												|
-----------------------------------------------------------------------------------*/
void LoRaAT::begin(uint32_t u32BaudRate) {
  switch(_u8SerialPort) {
  #if defined(UBRR1H)
    case 1:
    ATSerial = &Serial1;
    break;
  #endif
    
  #if defined(UBRR2H)
    case 2:
    ATSerial = &Serial2;
    break;
  #endif
    
  #if defined(UBRR3H)
    case 3:
    ATSerial = &Serial3;
    break;
  #endif
    
    case 0:
    default:
    ATSerial = &Serial;
    break;
  }

  ATSerial->begin(u32BaudRate);
}

/*----------------------------------------------------------------------------------|
| Initialize the mDot.																|
|																					|
| This method initilises the mDot with some default settings						|
-----------------------------------------------------------------------------------*/
int LoRaAT::init() {
  _debugStream->println("LaT:i : enter");

  return(0);
}

/*----------------------------------------------------------------------------------|
| Join a LoRa(WAN?) network															|
| 																					|
| TODO: Review returns, can we return something more meaningfull?					|
| Currently, zero is a success, negative integers are generic errors, and positive	|
| integers are a specific failure.													|
|   0 - success																		|
|   1 - mdot responds with failure to join network response							|
|  -1 - mdot responds with "ERROR"													|
|  -2 - Timeout error																|
| 																					|
| //TODO: parameters, saying something about network we want to join?				|
| 																					|
| uses a default of 10,000ms (10sec) timeout										|
-----------------------------------------------------------------------------------*/
int LoRaAT::join() {
  return(join(10000));
}

/*----------------------------------------------------------------------------------|
| Join a LoRa(WAN?) network															|
|																					| 
| TODO: Review returns, can we return something more meaningfull?					|
| Currently, zero is a success, negative integers are generic errors, and positive	|
| integers are a specific failure.													|
|   0 - success																		|
|   1 - mdot responds with failure to join network response							|
|  -1 - Timeout error																|
|  -2 - mdot responds with "ERROR"													|
| 																					|
| //TODO: parameters, saying something about network we want to join?				|
| 																					|
| takes the parameter timeout, which is the number of milliseconds you want it		|
| to wait for a response.															|
-----------------------------------------------------------------------------------*/
int LoRaAT::join(unsigned int timeout) {
  _debugStream->println("LaT:j : enter");

  const int LOOP_DELAY = 250;			//Millisecond delay between serial attempts
  unsigned long timeoutCounter = 0;		//We don't wait forever for a response
  char _recievedString[_MAX_MDOT_RESPONSE];  //String returned by device
  int available;						//Number of bytes in serial buffer

  //flush receive buffer before transmitting request
  while (ATSerial->read() != -1);
  
  //Send join request
  _debugStream->println("LaT:j : AT+JOIN");
  ATSerial->println("AT+JOIN");
  
  //Loop reading from serial buffer until we get either a recognisable error, or
  //success
  timeoutCounter = 0;
  delay(10000);

  _debugStream->println("LaT:j : wait");
  while(timeoutCounter < timeout) {
    //Blank string
    for (int i = 0; i < _MAX_MDOT_RESPONSE; i++) 
    {
      _recievedString[i] = '\0';
    }
    available = ATSerial->available();			//Check number of bytes available
	///_debugStream->println("LaT:j : ATSerial.available?");
    if (available) {
	  ///_debugStream->println("LaT:j : yes, received:");
      if(ATSerial->readBytesUntil('\0', _recievedString, available)) {
		///_debugStream->print("LaT:j : ");
        ///_debugStream->println(_recievedString);
	    ///_debugStream->println("LaT:j : keyword 'OK'?");
        if (strstr(_recievedString, "OK") != '\0') {
		  _debugStream->println("LaT:j : Keyword 'OK', return 0");
          return (0);
		}
      }
    }
	///_debugStream->println("LaT:j : no, delay/check again");
    timeoutCounter += LOOP_DELAY;
    delay(LOOP_DELAY);
  }

  _debugStream->println("LaT:j : timed-out. return -1");
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Leave a LoRa(WAN?) network														|
-----------------------------------------------------------------------------------*/
void LoRaAT::leave() {
  _debugStream->println("LaT:l: not implemented");
}

/*----------------------------------------------------------------------------------|
| In general we send strings using the AT command, TODO: we could have overloaded	|
| functions to accept other things?													|
|																					| 
| TODO: Input checking?																|
|   - Too long?																		|
|	  + Return error?																|
|	  + Break it up?																|
|	  + Send first part?															|
|	- Too short?																	|
|	  + Is it even a problem?														|
|	  + Pad it out?																	|
|	  + Return error?																|
|	- Invalid characters?															|
|	  + Is it even a problem?														|
|	  + Send anyway?																|
|	  + Replace with something else?												|
|	  + Return error?																|
| 																					|
| uses a default of 10,000ms (10sec) timeout										|
-----------------------------------------------------------------------------------*/
int LoRaAT::send(char* message) {
  ///_debugStream->println("LaT:s : enter, w/ default timeout");
  return(send(message,10000));
}

/*----------------------------------------------------------------------------------|
| In general we send strings using the AT command, TODO: we could have overloaded	|
| functions to accept other things?													|
|																					| 
| TODO: Input checking?																|
|   - Too long?																		|
|	  + Return error?																|
|	  + Break it up?																|
|	  + Send first part?															|
|	- Too short?																	|
|	  + Is it even a problem?														|
|	  + Pad it out?																	|
|	  + Return error?																|
|	- Invalid characters?															|
|	  + Is it even a problem?														|
|	  + Send anyway?																|
|	  + Replace with something else?												|
|	  + Return error?																|
-----------------------------------------------------------------------------------*/
int LoRaAT::send(char* message, unsigned int timeout) {
  _debugStream->println("LaT:s : enter");
  ///_debugStream->println("LaT:s : sending:");
  ///_debugStream->print("LaT:s : ");
  ///_debugStream->println(message);

  int LOOP_DELAY = 250;					//Millisecond delay between serial attempts
  unsigned long timeoutCounter = 0;		//We don't wait forever for a response
  char _recievedString[_MAX_MDOT_RESPONSE];	//String returned by device
  int available;						//Number of bytes in serial buffer
  
  //flush receive buffer before transmitting request
  while (ATSerial->read() != -1);
  
  //Mirror on debug what we're going to output on ATSerial
  _debugStream->print("LaT:s : ");
  _debugStream->print("AT+SEND ");
  for (int i = 0; i < _PACKET_SIZE; i++)
  {
    _debugStream->print(message[i]);
  }
  _debugStream->println();
  
  //Send message
  ATSerial->print("LaT:s : AT+SEND ");
  for (int i = 0; i < _PACKET_SIZE; i++)
  {
    ATSerial->print(message[i]);
  }
  ATSerial->println();
  
  //Loop reading from serial buffer until we get either a recognisable error, or
  //success
  timeoutCounter = 0;
  delay(2000);

  _debugStream->println("LaT:s : wait");
  while(timeoutCounter < timeout) {
	//Blank string
    for (int i = 0; i < _MAX_MDOT_RESPONSE; i++) 
    {
      _recievedString[i] = '\0';
    }
    available = ATSerial->available();
	///_debugStream->println("LaT:s : ATSerial.available?");
    if (available) {
	  ///_debugStream->println("LaT:s : yes:");
      if(ATSerial->readBytesUntil('\0', _recievedString, available)) 
      {
		///_debugStream->print("LaT:s : ");
        ///_debugStream->println(_recievedString);
		///_debugStream->println("LaT:s : keyword 'OK'?:");
        if (strstr(_recievedString, "OK") != '\0') {
		  _debugStream->println("LaT:s : Keyword 'OK', return 0");
          return (0);
		}
      }
    }
	///_debugStream->println("LaT:s : No, delay/check again");
    timeoutCounter += LOOP_DELAY;
    delay(LOOP_DELAY);
  }

  _debugStream->println("LaT:s : timed-out. return -1");
  return(-1);
}

/*----------------------------------------------------------------------------------|
| I believe there maybe a ping function... I'm not sure yet what the AT command		|
| returns, or what we should return to the user?									|
-----------------------------------------------------------------------------------*/
uint8_t LoRaAT::ping() {
  _debugStream->println("LaT:p: not implemented");
}

/*----------------------------------------------------------------------------------|
| Recieves a string in the format key:value,key:value,...							|
|																					|
| 1. The csv is translated to a json,												|
| 2. The json is added to the _txBuffer												|
| 3. The _txBuffer is processed														|
|																					|
| //TODO: If not recieved in that format return an error							|
-----------------------------------------------------------------------------------*/
int LoRaAT::sendPairs(String pairs) 
{
  ///_debugStream->println("LaT:sp: enter");
  char pairsC[_MAX_PAIRS_SIZE];
  ///_debugStream->println("LaT:sp: string to char[]");
  pairs.toCharArray(pairsC, _MAX_PAIRS_SIZE);
  LoRaAT::sendPairs(pairsC);
}

/*----------------------------------------------------------------------------------|
| Recieves a char array in the format key:value,key:value,...						|
|																					|
| 1. The csv is translated to a json,												|
| 2. The json is added to the _txBuffer												|
| 3. The _txBuffer is processed														|
|																					|
| //TODO: If not recieved in that format return an error							|
-----------------------------------------------------------------------------------*/
int LoRaAT::sendPairs(char* pairs) {
  _debugStream->println("LaT:sp: enter");
  ///_debugStream->println("LaT:sp: pairs:");
  ///_debugStream->print("LaT:sp: ");
  ///_debugStream->println(pairs);
  //Return constants
  const byte UNKNOWN_FORMAT = 4;
  int response = 0;

  //String json;      
  char json[_MAX_PAIRS_SIZE];      
  
  //TODO: Check the string is actually pairs
  if (false) { //TODO: if format not recoginised
    return(UNKNOWN_FORMAT);
  }

  ///_debugStream->println("LaT:sp: convert to JSON");
  _pairsToJSON(json, pairs);
  ///_debugStream->println("LaT:sp: pairs as JSON");
  ///_debugStream->print("LaT:sp: ");
  ///_debugStream->println(json);
  ///_debugStream->println("LaT:sp: fragment JSON to buffer");
  _createFragmentBuffer(json);
  ///_debugStream->println("LaT:sp: process buffer");
  response = _processBuffer();

  return(response);
}

/*----------------------------------------------------------------------------------|
| This function will take any correctly formatted char array of key:value pairs		|
| and return a JSON formatted String.												|
-----------------------------------------------------------------------------------*/
void LoRaAT::_pairsToJSON(char* json, char* pairs) {
  _debugStream->println("LaT:pj: enter");
  char* jsonPtr = json;
  char temp[20];
  int len = strlen(pairs);
  char* tempPtr = temp;

  for (jsonPtr = json; jsonPtr < json + sizeof(json); jsonPtr++) 
  {
    *jsonPtr= '\0';
  }

  for (tempPtr = temp; tempPtr < temp + sizeof(temp); tempPtr++) 
  {
    *tempPtr = '\0';
  }

  jsonPtr = json;
  tempPtr = temp;

  // Adds the first {
  *jsonPtr++ = '{';

  //Loop through each of the characters, when getting to a delimiter, act accordingly
  for (int i = 0; i < len; i++) 
  {
    char c = pairs[i];
    if (c == ':') 
    {
      *jsonPtr++ = '\"';
      int no = tempPtr-temp;
      strncpy(jsonPtr, temp, no);
      jsonPtr += no;
      *jsonPtr++ = '\"';
      *jsonPtr++ = ':';
      for (tempPtr = temp; tempPtr < temp + sizeof(temp); tempPtr++) 
      {
        *tempPtr = '\0';
      }
      tempPtr = temp;
    } 
    else if (c == ',') 
    {
      int no = tempPtr-temp;
      strncpy(jsonPtr, temp, no);
      jsonPtr += no;
      *jsonPtr++ = ',';
      for (tempPtr = temp; tempPtr < temp + sizeof(temp); tempPtr++) 
      {
        *tempPtr = '\0';
      }
      tempPtr = temp;
    } 
    else 
    {
      *tempPtr++ = c;
    }
  }
  int no = tempPtr-temp;
  strncpy(jsonPtr, temp, no);
  jsonPtr += no;
  //no comma at the end of last JSON value
  *jsonPtr++ = '}';
  *jsonPtr++ = '\0';

  ///_debugStream->println("LaT:pj: exit");
  return;
}

/*----------------------------------------------------------------------------------|
| This function will take and ASCII String message and fragment it into 11 byte		|
| packets.																			|
|																					|
| 2 bytes of header, and 9 bytes of payloads										|
|																					|
| header is of the format [fragment number][total number of fragments]				|
-----------------------------------------------------------------------------------*/
void LoRaAT::_createFragmentBuffer(char* message) {
  _debugStream->println("LaT:fb: enter");

  int strLength = strlen(message);
  //Calculate the number of fragments required for this message
  int numFragments = strLength/_PAYLOAD_SIZE;
  
  //Make sure we round up
  if (_PAYLOAD_SIZE*numFragments < strLength) {
    numFragments += 1;
  }

  //Check we haven't exceeded the maximum number of fragments
  if (numFragments > _MAX_FRAGMENTS) 
  {
    _debugStream->println("LaT:fb: max frags exceeded");
    numFragments = _MAX_FRAGMENTS;
  }

  ///_debugStream->print("LaT:fb: create ");
  ///_debugStream->print(numFragments);
  ///_debugStream->println(" fragments");
  //Loop through each fragment
  for (_txPutter = 0; _txPutter < numFragments; _txPutter++) {
    union headerPacket 
    {
      uint8_t asByte[2];
      char asChar[2];
    } header;

    //Create the header bytes
    header.asByte[0] = _txPutter + 1;
    header.asByte[1] = numFragments;

    //Add the header to the byte array (delt with as a string)
    _txBuffer[_txPutter][0] = header.asChar[0];
    _txBuffer[_txPutter][1] = header.asChar[1];

    ///_debugStream->print("LaT:fb: create fragment ");
	///_debugStream->println(_txPutter);
    //Loop through each location of the message and append to the fragment (as the payload)
    for (uint8_t j = 0; j < _PAYLOAD_SIZE; j++) 
    {
      _txBuffer[_txPutter][j + _HEADER_SIZE] = message[_PAYLOAD_SIZE * _txPutter + j];        
      if (message[_PAYLOAD_SIZE * _txPutter + j] == '\0') 
      {
        // padding the rest of the fragment if not fitting 100% with space
        while (j < _PAYLOAD_SIZE)
        {
          _txBuffer[_txPutter][j + _HEADER_SIZE]   = ' ';
          j++;
        }
        break;
      }
    }
	///_debugStream->print("LaT:fb: ");
	///_debugStream->println(_txBuffer[_txPutter]);
  }
}

/*----------------------------------------------------------------------------------|
| Buffer processing function, which will send out all data currently in the buffer  |
-----------------------------------------------------------------------------------*/
int LoRaAT::_processBuffer() 
{
  _debugStream->println("LaT:pb: enter");

  int response;
  for(_txGetter; _txGetter < _txPutter; _txGetter++ )
  {
	_debugStream->print("LaT:pb: putter: ");
	_debugStream->print(_txPutter);
	_debugStream->print(" getter: ");
	_debugStream->println(_txGetter);
	_debugStream->println("LaT:pb: packet as HEX:");
	_debugStream->print("LaT:pb: ");
	for (int j=0; j < _PACKET_SIZE; j++)
    {
      _debugStream->print(_txBuffer[_txGetter][j], HEX);
    }
	_debugStream->println();
    response = send(_txBuffer[_txGetter]);
    ///_debugStream->print("LaT:pb: sent. response: ");
    ///_debugStream->println(response, DEC);
  }
  _txPutter = 0;
  _txGetter = 0;
  return(response);
}

/*----------------------------------------------------------------------------------|
| Sets the frequency sub band                                                       |
-----------------------------------------------------------------------------------*/
int LoRaAT::setFrequencySubBand() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Gets the frequency sub band                                                       |
-----------------------------------------------------------------------------------*/
int LoRaAT::getFrequencySubBand() {
  _debugStream->println("LaT:gf: enter");

  static const uint16_t timeout = 10000;
  static const uint16_t LOOP_DELAY = 250;	//Millisecond delay between serial attempts
  
  unsigned long timeoutCounter = 0;		    //We don't wait forever for a response
  char _recievedString[_MAX_MDOT_RESPONSE];	//String returned by device
  int available;						    //Number of bytes in serial buffer
  
  //flush receive buffer before transmitting request
  while (ATSerial->read() != -1);
  
  //Mirror on debug what we're going to output on ATSerial
  _debugStream->println("LaT:gf: AT+FSB?");
  ATSerial->println("LaT:gf: AT+FSB?");
  
  //Loop reading from serial buffer until we get either a recognisable error, or
  //success
  timeoutCounter = 0;

  _debugStream->println("LaT:gf: wait");
  while(timeoutCounter < timeout) {
	//Blank string
    for (int i = 0; i < _MAX_MDOT_RESPONSE; i++) 
    {
      _recievedString[i] = '\0';
    }
    available = ATSerial->available();
	_debugStream->println("LaT:gf: ATSerial.available?");
    if (available) {
	  _debugStream->println("LaT:gf: yes:");
      if(ATSerial->readBytesUntil('\0', _recievedString, available)) 
      {
		_debugStream->print("LaT:gf: ");
        _debugStream->println(_recievedString);
		_debugStream->println("LaT:gf: keyword 'OK'?:");
        if (strstr(_recievedString, "OK") != '\0') {
		  _debugStream->println("LaT:gf: Keyword 'OK', return 0");
          return (0);
		}
      }
    }
	_debugStream->println("LaT:gf: No, delay/check again");
    timeoutCounter += LOOP_DELAY;
    delay(LOOP_DELAY);
  }

  _debugStream->println("LaT:s : timed-out. return -1");
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Sets the public network                                                           |
-----------------------------------------------------------------------------------*/
int LoRaAT::setPublicNetwork() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Sets the public network                                                           |
-----------------------------------------------------------------------------------*/
int LoRaAT::getPublicNetwork() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Sets the network ID                                                               |
-----------------------------------------------------------------------------------*/
int LoRaAT::setNetworkID() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Gets the network ID                                                               |
-----------------------------------------------------------------------------------*/
int LoRaAT::getNetworkID() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Sets the network key                                                              |
-----------------------------------------------------------------------------------*/
int LoRaAT::setNetworkKey() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Gets the network key                                                              |
-----------------------------------------------------------------------------------*/
int LoRaAT::getNetworkKey() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Sets the data rate                                                                |
|																					|
| this function can be used to set a specific data rate or set the data rate to     |
| adaptive.																			|
-----------------------------------------------------------------------------------*/
int LoRaAT::setDataRate() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Gets the data rate                                                                |
-----------------------------------------------------------------------------------*/
int LoRaAT::getDataRate() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Sets the ???                                                                      |
-----------------------------------------------------------------------------------*/
int LoRaAT::setRXO() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Gets the ???                                                                      |
-----------------------------------------------------------------------------------*/
int LoRaAT::getRXO() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Writes the current settings of the mDot to it's memory                            |
-----------------------------------------------------------------------------------*/
int LoRaAT::commitSettings() {

  return(0);
}
