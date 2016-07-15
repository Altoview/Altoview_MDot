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
  HardwareSerial* ATSerial = &Serial;            //Pointer to Serial class object
#elif defined(ARDUINO_ARCH_SAM)
  UARTClass* ATSerial = &Serial;                 //Pointer to Serial class object
#else
  #error "This library only supports boards with an AVR or SAM processor."
#endif


/************************************************************************************
 *                               PUBLIC FUNCTIONS                                   *
 ***********************************************************************************/

/*----------------------------------------------------------------------------------|
| CONSTRUCTOR: Creates class object using a default hardware serial port 0          |
-----------------------------------------------------------------------------------*/
 LoRaAT::LoRaAT() {
  _u8SerialPort = 0;
  _debugStream = NULL;
}

/*----------------------------------------------------------------------------------|
| CONSTRUCTOR: Creates class object using a specified serial port.                  |
-----------------------------------------------------------------------------------*/
 LoRaAT::LoRaAT(uint8_t u8SerialPort) {
  //TODO: Input checking, what range of values to accept, how to handle invalid input
  _u8SerialPort = u8SerialPort;
  _debugStream = NULL;
}

/*----------------------------------------------------------------------------------|
| CONSTRUCTOR: Creates class object using a specified serial port, and passing it a |
| specificed debug stream. This stream can be used to pass debug info to a serial   |
| port, either hardware serial or software serial.                                  |
-----------------------------------------------------------------------------------*/
LoRaAT::LoRaAT(uint8_t u8SerialPort, Stream* debugStream) {
  //TODO: Input checking, what range of values to accept, how to handle invalid input
  _u8SerialPort = u8SerialPort;
  _debugStream = debugStream;
}
  
/*----------------------------------------------------------------------------------|
| Initialize class object.                                                          |
|                                                                                   |
| Sets up the serial port using default 19200 baud rate.                            |
| Call once class has been instantiated, typically within setup().                  |
-----------------------------------------------------------------------------------*/
void LoRaAT::begin(void) {
  //TODO: Input checking??
  begin(19200);
}

/*----------------------------------------------------------------------------------|
| Initialize class object.                                                          |
|                                                                                   |
| Call once class has been instantiated, typically within setup().                  |
|                                                                                   |
| TODO: Think about what we should set here, what should be defaults. Datarates,    |
| adaptive data rates? Other things?                                                |
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
| the send command method, sends a command to the mDot and waits for a response.    |
| once a recognised response is received it returns the corresponding interger,     |
| if no recognised response is recieved in the timout specified return 0.           |
-----------------------------------------------------------------------------------*/
uint8_t LoRaAT::_sendCommand(char* command, char* ans1, char* ans2, char* ans3, char* ans4, uint16_t timeout) {
  return _sendCommand(command, ans1, ans2, ans3, ans4, timeout, NULL);
}
/*----------------------------------------------------------------------------------|
| the send command method, sends a command to the mDot and waits for a response.    |
| once a recognised response is received it returns the corresponding interger,     |
| if no recognised response is recieved in the timout specified return 0.           |
-----------------------------------------------------------------------------------*/
uint8_t LoRaAT::_sendCommand(char* command, char* ans1, char* ans2, char* ans3, char* ans4, uint16_t timeout, char** resp) {
  ///_debugStream->println(F("LaT:sc: enter"));
  static const char TERMINATOR[3] = {'\r','\n','\0'};
   
  unsigned long maxEndTime = 0;

  //flush receive buffer before transmitting request
  while (ATSerial->read() != -1);
  
  //Blank string
  memset(_response,0x00,_MAX_MDOT_RESPONSE);
  _length = 0;
  
  //Send command
  _debugStream->print(F("LaT:sc: "));
  _debugStream->print(command);
  _debugStream->print(TERMINATOR);
  ATSerial->print(command);
  ATSerial->print(TERMINATOR);
  
  //Set timeout time
  maxEndTime = millis() + timeout;
  
  //While something is available get it
  ///_debugStream->println(F("LaT:sc: Loop collecting response"));
  *resp = NULL;
  do {
    if (ATSerial->available() != 0) {
	  if (_length < (_MAX_MDOT_RESPONSE - 1)) {
        _response[_length++] = ATSerial->read();
	  }
    }
  
    if (strstr(_response, ans1) != '\0') {
      if (resp != NULL) {
        *resp = strstr(_response,command);
        *resp += strlen(command);
        *resp += sizeof(TERMINATOR);               //3
      }
      return (1);
    }
	
    if (strstr(_response, ans2) != '\0') {
      if (resp != NULL) {
        *resp = strstr(_response,command);
        *resp += strlen(command);
        *resp += sizeof(TERMINATOR);               //3
      }
      return (2);
    }
	
    if (strstr(_response, ans3) != '\0') {
      if (resp != NULL) {
        *resp = strstr(_response,command);
        *resp += strlen(command);
        *resp += sizeof(TERMINATOR);               //3
      }
      return (3);
    }
	
    if (strstr(_response, ans4) != '\0') {
      if (resp != NULL) {
        *resp = strstr(_response,command);
        *resp += strlen(command);
        *resp += sizeof(TERMINATOR);               //3
      }
      return (4);
    }
	
  } while (millis() <= maxEndTime);
  
  _debugStream->println(F("LaT:sc: Timed out"));
  return (0);
}

/*----------------------------------------------------------------------------------|
| Join a LoRa(WAN?) network                                                         |
|                                                                                   |
| TODO: Review returns, can we return something more meaningfull?                   |
| Currently, zero is a success, negative integers are generic errors, and positive  |
| integers are a specific failure.                                                  |
|   0 - success                                                                     |
|   1 - mdot responds with failure to join network response                         |
|  -1 - mdot responds with "ERROR"                                                  |
|  -2 - Timeout error                                                               |
|                                                                                   |
| //TODO: parameters, saying something about network we want to join?               |
|                                                                                   |
| uses a default of 10,000ms (10sec) timeout                                        |
-----------------------------------------------------------------------------------*/
/*int LoRaAT::join() {
  return(join(10000));
}

/*----------------------------------------------------------------------------------|
| Join a LoRa(WAN?) network                                                         |
|                                                                                   | 
| TODO: Review returns, can we return something more meaningfull?                   |
| Currently, zero is a success, negative integers are generic errors, and positive  |
| integers are a specific failure.                                                  |
|   0 - success                                                                     |
|   1 - mdot responds with failure to join network response                         |
|  -1 - Timeout error                                                               |
|  -2 - mdot responds with "ERROR"                                                  |
|                                                                                   |
| //TODO: parameters, saying something about network we want to join?               |
|                                                                                   |
| takes the parameter timeout, which is the number of milliseconds you want it      |
| to wait for a response.                                                           |
-----------------------------------------------------------------------------------*/
/*int LoRaAT::join(unsigned int timeout) {
  ///_debugStream->println(F("LaT:j : enter"));
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";

  sprintf_P(_command,(char*)F("AT+JOIN"));
  
  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,timeout);

  if (ansCode == 1) {
    return (0);
  }
  
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Leave a LoRa(WAN?) network                                                        |
-----------------------------------------------------------------------------------*/
/*void LoRaAT::leave() {
  ///_debugStream->println(F("LaT:l: not implemented"));
}

/*----------------------------------------------------------------------------------|
| In general we send strings using the AT command, TODO: we could have overloaded   |
| functions to accept other things?                                                 |
|                                                                                   | 
| TODO: Input checking?                                                             |
|   - Too long?                                                                     |
|      + Return error?                                                              |
|      + Break it up?                                                               |
|      + Send first part?                                                           |
|    - Too short?                                                                   |
|      + Is it even a problem?                                                      |
|      + Pad it out?                                                                |
|      + Return error?                                                              |
|    - Invalid characters?                                                          |
|      + Is it even a problem?                                                      |
|      + Send anyway?                                                               |
|      + Replace with something else?                                               |
|      + Return error?                                                              |
|                                                                                   |
| uses a default of 10,000ms (10sec) timeout                                        |
-----------------------------------------------------------------------------------*/
/*int LoRaAT::send(char* message) {
  ///_debugStream->println(F("LaT:s : enter, w/ default timeout"));
  return(send(message,10000));
}

/*----------------------------------------------------------------------------------|
| In general we send strings using the AT command, TODO: we could have overloaded   |
| functions to accept other things?                                                 |
|                                                                                   | 
| TODO: Input checking?                                                             |
|   - Too long?                                                                     |
|      + Return error?                                                              |
|      + Break it up?                                                               |
|      + Send first part?                                                           |
|    - Too short?                                                                   |
|      + Is it even a problem?                                                      |
|      + Pad it out?                                                                |
|      + Return error?                                                              |
|    - Invalid characters?                                                          |
|      + Is it even a problem?                                                      |
|      + Send anyway?                                                               |
|      + Replace with something else?                                               |
|      + Return error?                                                              |
-----------------------------------------------------------------------------------*/
/*int LoRaAT::send(char* message, unsigned int timeout) {
  ///_debugStream->println(F("LaT:s : enter"));
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";
  
  sprintf_P(_command,(char*)F("AT+SEND "));
  strcat(_command,message);                      //Append message to command

  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,timeout);

  if (ansCode == 1) {
    return (0);
  }
  
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Not yet implemented.                                                              |
-----------------------------------------------------------------------------------*/
/*uint8_t LoRaAT::ping() {
  ///_debugStream->println(F("LaT:p: not implemented"));
}

/*----------------------------------------------------------------------------------|
| Recieves a string in the format key:value,key:value,...                           |
|                                                                                   |
| 1. The csv is translated to a json,                                               |
| 2. The json is added to the _txBuffer                                             |
| 3. The _txBuffer is processed                                                     |
|                                                                                   |
| //TODO: If not recieved in that format return an error                            |
-----------------------------------------------------------------------------------*/
/*int LoRaAT::sendPairs(String pairs) 
{
  ///_debugStream->println(F("LaT:sp: enter"));
  char pairsC[_MAX_PAIRS_SIZE];
  ///_debugStream->println(F("LaT:sp: string to char[]"));
  pairs.toCharArray(pairsC, _MAX_PAIRS_SIZE);
  LoRaAT::sendPairs(pairsC);
}

/*----------------------------------------------------------------------------------|
| Recieves a char array in the format key:value,key:value,...                       |
|                                                                                   |
| 1. The csv is translated to a json,                                               |
| 2. The json is added to the _txBuffer                                             |
| 3. The _txBuffer is processed                                                     |
|                                                                                   |
| //TODO: If not recieved in that format return an error                            |
-----------------------------------------------------------------------------------*/
/*int LoRaAT::sendPairs(char* pairs) {
  ///_debugStream->println(F("LaT:sp: enter"));
  ///_debugStream->println(F("LaT:sp: pairs:"));
  ///_debugStream->print(F("LaT:sp: "));
  ///_debugStream->println(pairs);
  //Return constants
  const byte UNKNOWN_FORMAT = 4;
  int response = 0;

  //String json;      
  char json[_MAX_PAIRS_SIZE];      
  
  //TODO: Check the string is actually pairs
  if (false) { //TODO: if format not recognised
    return(UNKNOWN_FORMAT);
  }

  ///_debugStream->println(F("LaT:sp: convert to JSON"));
  _pairsToJSON(json, pairs);
  ///_debugStream->println(F("LaT:sp: pairs as JSON"));
  _debugStream->print(F("LaT:sp: "));
  _debugStream->println(json);
  ///_debugStream->println(F("LaT:sp: fragment JSON to buffer"));
  _createFragmentBuffer(json);
  ///_debugStream->println(F("LaT:sp: process buffer"));
  response = _processBuffer();

  return(response);
}

/*----------------------------------------------------------------------------------|
| This function will take any correctly formatted char array of key:value pairs     |
| and return a JSON formatted String.                                               |
-----------------------------------------------------------------------------------*/
/*void LoRaAT::_pairsToJSON(char* json, char* pairs) {
  ///_debugStream->println(F("LaT:pj: enter"));
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

  ///_debugStream->println(F("LaT:pj: exit"));
  return;
}

/*----------------------------------------------------------------------------------|
| This function will take and ASCII String message and fragment it into 11 byte     |
| packets.                                                                          |
|                                                                                   |
| 2 bytes of header, and 9 bytes of payloads                                        |
|                                                                                   |
| header is of the format [fragment number][total number of fragments]              |
-----------------------------------------------------------------------------------*/
/*void LoRaAT::_createFragmentBuffer(char* message) {
  ///_debugStream->println(F("LaT:fb: enter"));

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
    ///_debugStream->println(F("LaT:fb: max frags exceeded"));
    numFragments = _MAX_FRAGMENTS;
  }

  ///_debugStream->print(F("LaT:fb: create "));
  ///_debugStream->print(numFragments);
  ///_debugStream->println(F(" fragments"));
  //Loop through each fragment
  for (_txPutter = 0; _txPutter < numFragments; _txPutter++) {
    union headerPacket 
    {
      uint8_t asByte[2];
      char asChar[2];
    } header;

    //Create the header bytes
    header.asByte[0] = _txPutter + 1 + _HEADER_OFFSET;
    header.asByte[1] = numFragments + _HEADER_OFFSET;

    //Add the header to the byte array (delt with as a string)
    _txBuffer[_txPutter][0] = header.asChar[0];
    _txBuffer[_txPutter][1] = header.asChar[1];

    ///_debugStream->print(F("LaT:fb: create fragment "));
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
    ///_debugStream->print(F("LaT:fb: "));
    ///_debugStream->println(_txBuffer[_txPutter]);
  }
}

/*----------------------------------------------------------------------------------|
| Buffer processing function, which will send out all data currently in the buffer  |
-----------------------------------------------------------------------------------*/
/*int LoRaAT::_processBuffer() {
  char temp[_MAX_MDOT_COMMAND - 8];
  ///_debugStream->println(F("LaT:pb: enter"));

  int response;
  for(_txGetter; _txGetter < _txPutter; _txGetter++ )
  {
    ///_debugStream->print(F("LaT:pb: putter: "));
    ///_debugStream->print(_txPutter);
    ///_debugStream->print(F(" getter: "));
    ///_debugStream->println(_txGetter);
    ///_debugStream->println(F("LaT:pb: packet as HEX:"));
    ///_debugStream->print(F("LaT:pb: "));
    for (int j=0; j < _PACKET_SIZE; j++)
    {
      ///_debugStream->print(_txBuffer[_txGetter][j], HEX);
      temp[j] = _txBuffer[_txGetter][j];
      temp[j+1] = '\0';
    }
    ///_debugStream->println();
    response = send(temp);
    ///_debugStream->print(F("LaT:pb: sent. response: "));
    ///_debugStream->println(response, DEC);
  }
  _txPutter = 0;
  _txGetter = 0;
  return(response);
}


/*----------------------------------------------------------------------------------|
| Sets the frequency sub band                                                       |
|                                                                                   |
| AT+FSB ?                                                                          |
| AT+FSB: (0-8)                                                                     |
|                                                                                   |
| TODO:                                                                             |
|  * Overload to accept, string, int, uint, byte, maybe others, maybe less.         |
-----------------------------------------------------------------------------------*/
int LoRaAT::setFrequencySubBand(char fsb) {
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";
  
  sprintf_P(_command,(char*)F("AT+FSB "));
  _command[7] = fsb;
  _command[8] = '\0';

  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000);
  ///_debugStream->println(_response);

  if (ansCode == 1) {
	frequencySubBand = fsb;
    return (0);
  }
  
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the frequency sub band                                                       |
-----------------------------------------------------------------------------------*/
int LoRaAT::getFrequencySubBand() {
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";
  char* r;
  
  sprintf_P(_command,(char*)F("AT+FSB?"));

  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000, &r);
  
  if (ansCode == 1) {
	frequencySubBand = r[0];
    return (0);
  }
  
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Sets the public network                                                           |
|                                                                                   |
| AT+PN ?                                                                           |
| AT+PN: (0,1)                                                                      |
|  0 - off                                                                          |
|  1 - on                                                                           |
-----------------------------------------------------------------------------------*/
int LoRaAT::setPublicNetwork(char pn) {
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";
  
  sprintf_P(_command,(char*)F("AT+PN "));
  if (pn != '0') {
    pn = '1';
  }
  _command[6] = pn;
  _command[7] = '\0';
  
  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000);

  if (ansCode == 1) {
	publicNetwork = pn;
    return (0);
  }
  
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the public network                                                           |
-----------------------------------------------------------------------------------*/
int LoRaAT::getPublicNetwork() {
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";
  char* r;
  
  sprintf_P(_command,(char*)F("AT+PN?"));
  
  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000, &r);

  if (ansCode == 1) {
    publicNetwork = r[0];
    return (0);
  }
  
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Sets the network ID                                                               |
|                                                                                   |
| AT+NI ?                                                                           |
| AT+NI: (0,(hex:8)),(1,(string:128))                                               |
|                                                                                   |
| Examples:                                                                         |
|  * AT+NI 0,00:00:aa:00:00:00:00:01                                                |
|  * AT+NI 0,00-00-aa-00-00-00-00-01                                                |
-----------------------------------------------------------------------------------*/
int LoRaAT::setNetworkID(char* id) {
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";
  
  sprintf_P(_command,(char*)F("AT+NI 0,"));
  strcat(_command,id);                           //Append ID to command
  
  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000);

  if (ansCode == 1) {
	strncpy(networkId,id,sizeof(networkId)-1);
	networkId[23] = '\0';
    return (0);
  }
  
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the network ID                                                               |
-----------------------------------------------------------------------------------*/
int LoRaAT::getNetworkID() {
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";
  char* r;
  
  sprintf_P(_command,(char*)F("AT+NI?"));
  
  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000, &r);

  if (ansCode == 1) {
	strncpy(networkId,r,sizeof(networkId)-1);
	networkId[23] = '\0';
    return (0);
  }
  
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Sets the network key                                                              |
|                                                                                   |
| AT+NK ?                                                                           |
| AT+NK: (0,(hex:16)),(1,(string:128))                                              |
|                                                                                   |
| Examples:                                                                         |
|  * AT+NK 0,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01                        |
|  * AT+NK 0,00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-01                        |
-----------------------------------------------------------------------------------*/
int LoRaAT::setNetworkKey(char* key) {
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";
  
  sprintf_P(_command,(char*)F("AT+NK 0,"));
  strcat(_command,key);                          //Append key to command
  
  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000);

  if (ansCode == 1) {
	strncpy(networkKey,key,sizeof(networkKey)-1);
	networkKey[47] = '\0';
    return (0);
  }
  
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the network key                                                              |
-----------------------------------------------------------------------------------*/
int LoRaAT::getNetworkKey() {
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";
  char* r;
  
  sprintf_P(_command,(char*)F("AT+NK?"));
  
  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000, &r);

  if (ansCode == 1) {
	strncpy(networkKey,r,(sizeof(networkKey)-1));
	networkKey[47] = '\0';
    return (0);
  }
  
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Sets the data rate                                                                |
|                                                                                   |
| AT+TXDR ?                                                                         |
| AT+TXDR: (0-3|10-7|DR0-DR4|DR8-DR13)                                              |
-----------------------------------------------------------------------------------*/
int LoRaAT::setDataRate(char txdr) {
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";
  
  sprintf_P(_command,(char*)F("AT+TXDR "));
  _command[9] = txdr;
  _command[10] = '\0';
  
  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000);

  if (ansCode == 1) {
	dataRate = txdr;
    return (0);
  }
  
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the data rate                                                                |
-----------------------------------------------------------------------------------*/
int LoRaAT::getDataRate() {
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";
  char* r;
  
  sprintf_P(_command,(char*)F("AT+TXDR?"));
  
  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000, &r);

  if (ansCode == 1) {
	dataRate = r[0];
    return (0);
  }
  
  return(-1);
}

/*----------------------------------------------------------------------------------|
| Writes the current settings of the mDot to it's memory                            |
-----------------------------------------------------------------------------------*/
int LoRaAT::commitSettings() {
  uint8_t ansCode;
  char ans1[] PROGMEM = "OK";
  
  sprintf_P(_command,(char*)F("AT&W"));
  
  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000);

  if (ansCode == 1) {
    return (0);
  }
  
  return(-1);
}