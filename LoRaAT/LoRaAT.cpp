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
| the send command method, sends a command to the mDot and waits for a response.    |
| once a full response has been received it passes it back to the calling function  |
| as a char array.																	|
-----------------------------------------------------------------------------------*/
void LoRaAT::_sendCommand(char* response, char* command, uint16_t timeout) {
  ///_debugStream->println("LaT:sc: enter");
  
  static const uint16_t LOOP_DELAY = 250;	 //Millisecond delay between serial attempts
  
  unsigned long timeoutCounter = 0;		     //We don't wait forever for a response
  unsigned long maxEndTime = 0;				 //
  uint16_t i = 0;						     //Recieve character putter
  int available;						     //Number of bytes in serial buffer

  //flush receive buffer before transmitting request
  while (ATSerial->read() != -1);
  
  //Send command
  _debugStream->print("LaT:sc: ");
  _debugStream->println(command);
  ATSerial->println(command);
    
  //Blank string
  for (int i = 0; i < _MAX_MDOT_RESPONSE; i++) {
    response[i] = '\0';
  }
  
  //Set timeout time
  maxEndTime = millis() + timeout;
  
  //Wait for something to be available
  ///_debugStream->println("LaT:sc: wait");
  do {
	if (millis() > maxEndTime) {
	  _debugStream->println("LaT:sc: timed-out");
	  return;
	}
  } while (ATSerial->available() == 0);
  
  //While something is available get it
  do {
	response[i] = ATSerial->read();
	i++;
	if (millis() > maxEndTime) {
	  _debugStream->println("LaT:sc: timed-out");
	  return;
	}
	delay(10);
  } while (ATSerial->available());
  
  _debugStream->println("LaT:sc: response received\r\n");
  _debugStream->println(response);
  return;
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
  char _recievedString[_MAX_MDOT_RESPONSE];  //String returned by device
  char command[] = "AT+JOIN";
  
  _sendCommand(_recievedString,command,timeout);

  if (strstr(_recievedString, "OK") != '\0') {
    _debugStream->println("LaT:j : Keyword 'OK', return 0");
    return (0);
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

  char _recievedString[_MAX_MDOT_RESPONSE];	//String returned by device
  char command[_MAX_MDOT_COMMAND] = "AT+SEND ";
 
  strcat(command,message);				    //Append message to command
  
  _sendCommand(_recievedString,command,timeout);

  if (strstr(_recievedString, "OK") != '\0') {
    _debugStream->println("LaT:s : Keyword 'OK', return 0");
    return (0);
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
int LoRaAT::_processBuffer() {
  char temp[_MAX_MDOT_COMMAND - 8];
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
	  temp[j] = _txBuffer[_txGetter][j];
	  temp[j+1] = '\0';
    }
	_debugStream->println();
    response = send(temp);
    ///_debugStream->print("LaT:pb: sent. response: ");
    ///_debugStream->println(response, DEC);
  }
  _txPutter = 0;
  _txGetter = 0;
  return(response);
}

/*----------------------------------------------------------------------------------|
| Sets the frequency sub band                                                       |
|																					|
| TODO:																				|
|  * parse the response																|
|  * Return something meaningfull (based on response)								|
|  * Overload to accept, string, int, uint, byte, maybe others, maybe less.			|
-----------------------------------------------------------------------------------*/
int LoRaAT::setFrequencySubBand(char fsb) {
  ///_debugStream->println("LaT:gf: enter");
  static const uint16_t timeout = 10000;	  //Max time allowed to receive response
  static const uint8_t _MAX_FSB_COMMAND = 9;  //Max characters in FSB command "AT+FSB x"
  char command[_MAX_FSB_COMMAND] = "AT+FSB "; //Command to get frequency sub band
  char _recievedString[_MAX_MDOT_RESPONSE];	  //String returned by device
  
  command[7] = fsb;
  command[8] = '\0';
  
  _sendCommand(_recievedString,command,timeout);

  if (strstr(_recievedString, "OK") != '\0') {
    ///_debugStream->println("LaT:gf: Keyword 'OK', return 0");
    return (0);
  }

  ///_debugStream->println("LaT:gf: timed-out. return -1");
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the frequency sub band                                                       |
|																					|
| TODO:																				|
|  * parse the response																|
|  * Return something meaningfull (based on response)								|
-----------------------------------------------------------------------------------*/
int LoRaAT::getFrequencySubBand() {
  ///_debugStream->println("LaT:gf: enter");
  static const uint16_t timeout = 10000;	//Max time allowed to receive response
  
  char command[] = "AT+FSB?";				//Command to get frequency sub band
  char _recievedString[_MAX_MDOT_RESPONSE];	//String returned by device
  
  _sendCommand(_recievedString,command,timeout);

  if (strstr(_recievedString, "OK") != '\0') {
    ///_debugStream->println("LaT:gf: Keyword 'OK', return 0");
    return (0);
  }

  ///_debugStream->println("LaT:gf: timed-out. return -1");
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Sets the public network                                                           |
-----------------------------------------------------------------------------------*/
int LoRaAT::setPublicNetwork() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Gets the public network                                                           |
|																					|
| TODO:																				|
|  * parse the response																|
|  * Return something meaningfull (based on response)								|
-----------------------------------------------------------------------------------*/
int LoRaAT::getPublicNetwork() {
  ///_debugStream->println("LaT:gp: enter");
  static const uint16_t timeout = 10000;	//Max time allowed to receive response
  
  char command[] = "AT+PN?";				//Command to get public network
  char _recievedString[_MAX_MDOT_RESPONSE];	//String returned by device
  
  _sendCommand(_recievedString,command,timeout);

  if (strstr(_recievedString, "OK") != '\0') {
    ///_debugStream->println("LaT:gp: Keyword 'OK', return 0");
    return (0);
  }

  ///_debugStream->println("LaT:gp: timed-out. return -1");
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Sets the network ID                                                               |
-----------------------------------------------------------------------------------*/
int LoRaAT::setNetworkID() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Gets the network ID                                                               |
|																					|
| TODO:																				|
|  * parse the response																|
|  * Return something meaningfull (based on response)								|
-----------------------------------------------------------------------------------*/
int LoRaAT::getNetworkID() {
  ///_debugStream->println("LaT:gi: enter");
  static const uint16_t timeout = 10000;	//Max time allowed to receive response
  
  char command[] = "AT+NI?";				//Command to get network ID
  char _recievedString[_MAX_MDOT_RESPONSE];	//String returned by device
  
  _sendCommand(_recievedString,command,timeout);

  if (strstr(_recievedString, "OK") != '\0') {
    ///_debugStream->println("LaT:gi: Keyword 'OK', return 0");
    return (0);
  }

  ///_debugStream->println("LaT:gi: timed-out. return -1");
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Sets the network key                                                              |
-----------------------------------------------------------------------------------*/
int LoRaAT::setNetworkKey() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Gets the network key                                                              |
|																					|
| TODO:																				|
|  * parse the response																|
|  * Return something meaningfull (based on response)								|
-----------------------------------------------------------------------------------*/
int LoRaAT::getNetworkKey() {
  ///_debugStream->println("LaT:gk: enter");
  static const uint16_t timeout = 10000;	//Max time allowed to receive response
  
  char command[] = "AT+NK?";				//Command to get the network key
  char _recievedString[_MAX_MDOT_RESPONSE];	//String returned by device
  
  _sendCommand(_recievedString,command,timeout);

  if (strstr(_recievedString, "OK") != '\0') {
    ///_debugStream->println("LaT:gk: Keyword 'OK', return 0");
    return (0);
  }

  ///_debugStream->println("LaT:gk: timed-out. return -1");
  return(-1);
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
|																					|
| TODO:																				|
|  * parse the response																|
|  * Return something meaningfull (based on response)								|
-----------------------------------------------------------------------------------*/
int LoRaAT::getDataRate() {
  ///_debugStream->println("LaT:gd: enter");
  static const uint16_t timeout = 10000;	//Max time allowed to receive response
  
  char command[] = "AT+ADR?";				//Command to get data rate
  char _recievedString[_MAX_MDOT_RESPONSE];	//String returned by device
  
  _sendCommand(_recievedString,command,timeout);

  if (strstr(_recievedString, "OK") != '\0') {
    ///_debugStream->println("LaT:gd: Keyword 'OK', return 0");
    return (0);
  }

  ///_debugStream->println("LaT:gd: timed-out. return -1");
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Sets the ???                                                                      |
-----------------------------------------------------------------------------------*/
int LoRaAT::setRXO() {

  return(0);
}

/*----------------------------------------------------------------------------------|
| Gets the ???                                                                      |
|																					|
| TODO:																				|
|  * parse the response																|
|  * Return something meaningfull (based on response)								|
-----------------------------------------------------------------------------------*/
int LoRaAT::getRXO() {
  ///_debugStream->println("LaT:gr: enter");
  static const uint16_t timeout = 10000;	//Max time allowed to receive response
  
  char command[] = "AT+RXO?";				//Command to get RXO
  char _recievedString[_MAX_MDOT_RESPONSE];	//String returned by device
  
  _sendCommand(_recievedString,command,timeout);

  if (strstr(_recievedString, "OK") != '\0') {
    ///_debugStream->println("LaT:gr: Keyword 'OK', return 0");
    return (0);
  }

  ///_debugStream->println("LaT:gr: timed-out. return -1");
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Writes the current settings of the mDot to it's memory                            |
-----------------------------------------------------------------------------------*/
int LoRaAT::commitSettings() {

  return(0);
}
