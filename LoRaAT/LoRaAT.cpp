/*
  File: LoRaAT.cpp

  Version: v0.0.1.1

  Brief: Arduino library for controlling Multitech mDot LoRa modules using
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

#define DEBUG

const char answer1[] = "OK";
const char answerX[] = "BUG";

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
| specified debug stream. This stream can be used to pass debug info to a serial    |
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
| Sets up the serial port specified in instantiation using default 38400 baud rate. |
| Then configure the mDot with some default settings.                               |
|                                                                                   |
| Call once class has been instantiated, typically within setup().                  |
-----------------------------------------------------------------------------------*/
void LoRaAT::begin() {
  begin(38400);
}

/*----------------------------------------------------------------------------------|
| Initialize class object.                                                          |
|                                                                                   |
| Sets up the serial port specified in instantiation with a user specified baud     |
| rate. Then configure the mDot with some default settings. If the serial port      |
| specific is not a valid one the serial port with default to 0.                    |
|                                                                                   |
| Call once class has been instantiated, typically within setup().                  |
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

  setDefaults();
}

/*----------------------------------------------------------------------------------|
| The send command method, takes a null terminated command, some responses, and a   |
| timeout.                                                                          |
|                                                                                   |
| The command is sent to the mDot and waits for a response. once a recognised       |
| response is received it returns the corresponding integer. If no recognised       |
| response is received within the timeout period a -1 is returned.                  |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::_sendCommand(char* command, char* ans1, char* ans2, char* ans3, char* ans4, uint16_t timeout) {
  return _sendCommand(command, ans1, ans2, ans3, ans4, timeout, NULL);
}
/*----------------------------------------------------------------------------------|
| The send command method, takes a null terminated command, some responses, a       |
| timeout and a pointer to a response.                                              |
|                                                                                   |
| The command is sent to the mDot and waits for a response. once a recognised       |
| response is received it returns the corresponding integer, and can return a       |
| pointer to the beginning of the actual response. If no recognised response is     |
| received within the timeout period a -1 is returned.                              |
|                                                                                   |
| The “actual response” is considered to begin just after the echoed termination of |
| the command. If the command is not found in the mDot response, the “actual        |
| response” is set to null.                                                         |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::_sendCommand(char* command, char* ans1, char* ans2, char* ans3, char* ans4, uint16_t timeout, char** resp) {
  ///_debugStream->println(F("LaT:sc: enter"));
  static const char TERMINATOR[3] = {'\r','\n','\0'};

  uint32_t maxEndTime = 0;

  //flush receive buffer before transmitting request
  delay(20);                                     //Undesirable dealy, if we read/write too quick to the mDot. We get out of time.
  while (ATSerial->read() != -1);
  delay(20);                                     //Undesirable dealy, if we read/write too quick to the mDot. We get out of time.

  //Blank string
  memset(_response,0x00,_MAX_MDOT_RESPONSE);
  _length = 0;
#ifdef DEBUG
  //Send command
  _debugStream->print(F("LaT:sc: "));
  _debugStream->print(command);
  _debugStream->print(TERMINATOR);
#endif
  ATSerial->print(command);
  ATSerial->print(TERMINATOR);

  //Set timeout time
  maxEndTime = millis() + timeout;

  //While something is available get it
  ///_debugStream->println(F("LaT:sc: Loop collecting response"));
  *resp = NULL;
  do {
    if (ATSerial->available() != 0) {
      if (_length < (_MAX_MDOT_RESPONSE - 2)) {
        _response[_length++] = ATSerial->read();
        _response[_length] = '\0';               //Ensure response buffer is null terminated
      }
    }

    if (ans1 != NULL && strstr(_response, ans1) != '\0') {
      if (resp != NULL) {
        *resp = strstr(_response,command);
        *resp += strlen(command);
        *resp += sizeof(TERMINATOR);
      }
      //_debugStream->print(F("LaT:sc: "));
      //_debugStream->println(_response);
      return (1);
    }

    if (ans2 != NULL && strstr(_response, ans2) != '\0') {
      if (resp != NULL) {
        *resp = strstr(_response,command);
        *resp += strlen(command);
        *resp += sizeof(TERMINATOR);
      }
      return (2);
    }

    if (ans3 != NULL && strstr(_response, ans3) != '\0') {
      if (resp != NULL) {
        *resp = strstr(_response,command);
        *resp += strlen(command);
        *resp += sizeof(TERMINATOR);
      }
      return (3);
    }

    if (ans4 != NULL && strstr(_response, ans4) != '\0') {
      if (resp != NULL) {
        *resp = strstr(_response,command);
        *resp += strlen(command);
        *resp += sizeof(TERMINATOR);
      }
      return (4);
    }

  } while (millis() <= maxEndTime);

  ///_debugStream->println(F("LaT:sc: Timed out"));
  return (-1);
}

/*----------------------------------------------------------------------------------|
| Join a LoRa(WAN?) network                                                         |
|                                                                                   |
| Uses a default of 10,000ms (10sec) timeout                                        |
|                                                                                   |
| Returns                                                                           |
|   0 - success                                                                     |
|  -1 - Timeout error                                                               |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::join() {
  return(join(10000));
}

/*----------------------------------------------------------------------------------|
| Join a LoRa(WAN?) network                                                         |
|                                                                                   |
| Takes the parameter timeout, which is the number of milliseconds to wait for a    |
| response.                                                                         |
|                                                                                   |
| Returns                                                                           |
|   0 - success                                                                     |
|  -1 - Timeout error                                                               |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::join(uint16_t timeout) {
  ///_debugStream->println(F("LaT:j : enter"));
  int8_t ansCode;

  sprintf_P(_command,(char*)F("AT+JOIN"));

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,timeout);
  if (ansCode < 0 ) {
    return(-1);
  }

  return(0);
}

/*----------------------------------------------------------------------------------|
| Leave a LoRa(WAN?) network                                                        |
-----------------------------------------------------------------------------------*/
void LoRaAT::leave() {
  ///_debugStream->println(F("LaT:l: not implemented"));
}

/*----------------------------------------------------------------------------------|
| Send method, takes a null terminated char array and sends that over the LoRaWAN   |
| network. Using a default timeout of 10,000ms (10sec).                             |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::send(char* message) {
  ///_debugStream->println(F("LaT:s : enter, w/ default timeout"));
  return(send(message,10000));
}

/*----------------------------------------------------------------------------------|
| Send method, takes a null terminated char array and sends that over the LoRaWAN   |
| network. Using a specified timeout.                                               |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::send(char* message, uint16_t timeout) {
  ///_debugStream->println(F("LaT:s : enter"));
  uint8_t l;

  l = sizeof(_command);                     //Will only send char array up to null, or l (max size of _command buffer)

  return(send(message,l,10000));
}

/*----------------------------------------------------------------------------------|
| Send method, takes a char array and length and sends that over the LoRaWAN        |
| network. Using a specified timeout.                                               |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::send(char* message, uint8_t length, uint16_t timeout) {
  ///_debugStream->println(F("LaT:s : enter"));
  int8_t ansCode;

  sprintf_P(_command,(char*)F("AT+SEND "));
  strncat(_command,message,length);              //Append message to command

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,timeout);

  if (ansCode == 1) {
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Not yet implemented.                                                              |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::ping() {
  ///_debugStream->println(F("LaT:p: not implemented"));
}

/*----------------------------------------------------------------------------------|
| Receives a string in the format key:value,key:value,...                           |
|                                                                                   |
| 1. The csv is translated to a JSON.                                               |
| 2. The json is fragmented to the _txBuffer .                                      |
| 3. The _txBuffer is processed (sent).                                             |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::sendPairs(String pairs) {
  return(sendPairs(&pairs));
}

/*----------------------------------------------------------------------------------|
| Receives a string in the format key:value,key:value,...                           |
|                                                                                   |
| 1. The csv is translated to a JSON.                                               |
| 2. The json is fragmented to the _txBuffer .                                      |
| 3. The _txBuffer is processed (sent).                                             |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::sendPairs(String* pairs) {
  ///_debugStream->println(F("LaT:sp: enter"));
  char pairsC[_MAX_PAIRS_SIZE];
  pairs->toCharArray(pairsC, _MAX_PAIRS_SIZE);
  pairsC[_MAX_PAIRS_SIZE-1] = '\0';
  return(sendPairs(pairsC));
}

/*----------------------------------------------------------------------------------|
| Receives a null terminated char array in the format key:value,key:value,...       |
|                                                                                   |
| 1. The csv is translated to a JSON.                                               |
| 2. The json is fragmented to the _txBuffer .                                      |
| 3. The _txBuffer is processed (sent).                                             |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::sendPairs(char* pairs) {
#ifdef DEBUG
  ///_debugStream->println(F("LaT:sp: enter"));
  _debugStream->print(F("LaT:sp: "));
  _debugStream->println(pairs);
#endif
  //String json;
  char json[_MAX_PAIRS_SIZE];
  memcpy(json,0x00,_MAX_PAIRS_SIZE);

  //TODO: Check the string is actually pairs

  ///_debugStream->println(F("LaT:sp: convert to JSON"));
  _pairsToJSON(json, _MAX_PAIRS_SIZE, pairs);
#ifdef DEBUG
  _debugStream->print(F("LaT:sp: "));
  _debugStream->println(json);
  ///_debugStream->println(F("LaT:sp: fragment JSON to buffer"));
#endif
  _createFragmentBuffer(json);
  ///_debugStream->println(F("LaT:sp: process buffer"));

  return(_processBuffer());
}

/*----------------------------------------------------------------------------------|
| This function will take any correctly formatted null terminated char array of     |
| key:value pairs and pass back a JSON formatted char array with a maximum size of  |
| jsonLength.                                                                       |
|                                                                                   |
| In the case the maximum JSON length is reached, the loop will exit, any partially |
| created pair will be removed, and the JSON closed and returned.                   |
-----------------------------------------------------------------------------------*/
void LoRaAT::_pairsToJSON(char* json, uint8_t jsonLength, char* pairs) {
  ///_debugStream->println(F("LaT:pj: enter"));
  char* jsonPtr;                                 //Points to the next free location
  uint8_t len = strlen(pairs);                   //Counts to the null terminator

  static const char JSON_BEGIN[2] = {'{','\"'};
  static const char JSON_STR_VAL[2] = {'\"',':'};
  static const char JSON_PAIR_PAIR[2] = {',','\"'};
  static const char JSON_END[2] = {'}','\0'};

  //Set the pointer to beginning of JSON
  jsonPtr = json;

  // Adds the first { and "
  memcpy(jsonPtr,JSON_BEGIN,sizeof(JSON_BEGIN));
  jsonPtr += sizeof(JSON_BEGIN);

  //Loop through each of the characters, when getting to a delimiter, act accordingly
  for (uint8_t j = 0; j < len && (jsonPtr - json + sizeof(JSON_END)) < jsonLength; j++) {
    char c = pairs[j];
    switch (c) {
      case ':':
        if (jsonPtr - json + sizeof(JSON_STR_VAL) < jsonLength) {
          memcpy(jsonPtr,JSON_STR_VAL,sizeof(JSON_STR_VAL));
          jsonPtr += sizeof(JSON_STR_VAL);
        }
        break;
      case ',':
        if (jsonPtr - json + sizeof(JSON_PAIR_PAIR) < jsonLength) {
          memcpy(jsonPtr,JSON_PAIR_PAIR,sizeof(JSON_PAIR_PAIR));
          jsonPtr += sizeof(JSON_PAIR_PAIR);
        }
        break;
      default:
        if ((jsonPtr - json) < jsonLength) {
          *jsonPtr++ = c;
        }
    }
  }

  //If we can't fit the JSON termination i.e. }\0, make room, then check for a partial pair, removing it if exists
  if ((jsonPtr - json + sizeof(JSON_END)) >= jsonLength) {
    jsonPtr -= sizeof(JSON_END);
    //"jsonPtr > json" so not to delete the open curly brace '{'
    while (jsonPtr - 1 > json && *jsonPtr != ',') {
      *jsonPtr-- = '\0';
    }
  }

  memcpy(jsonPtr,JSON_END,sizeof(JSON_END));
  jsonPtr += sizeof(JSON_END);

  ///_debugStream->println(F("LaT:pj: exit"));
  return;
}

/*----------------------------------------------------------------------------------|
| This function will take and null terminated ASCII char array message and fragment |
| it into 11 byte packets.                                                          |
|                                                                                   |
| 2 bytes of header, and 9 bytes of payload.                                        |
|                                                                                   |
| Header is of the format [fragment number][total number of fragments]              |
-----------------------------------------------------------------------------------*/
void LoRaAT::_createFragmentBuffer(char* message) {
  ///_debugStream->println(F("LaT:fb: enter"));

  uint8_t strLength = strlen(message);            //Counts to the null terminator
  //Calculate the number of fragments required for this message
  uint8_t numFragments = strLength/_PAYLOAD_SIZE;

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
          _txBuffer[_txPutter][j + _HEADER_SIZE]   = '\0';
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
|                                                                                   |
| This method gets a pointer to the fragment buffer, it then checks the mDot data   |
| rate, uses that value to set the length of bytes to send (from the fragment       |
| buffer). It checks to see if it will read past the end of the buffer, if it does  |
| it will reduce length to read only to the end of the buffer.                      |
|                                                                                   |
| The send function is then called with a reference to where in the fragment buffer |
| to start from and the number of bytes to send. After the send the pointer in the  |
| fragment buffer is updated. If we are pointing to the end of the buffer (or past  |
| it, which should never happen if we ever point past it, the length gets reduced   |
| accordingly) the loop is exited.                                                  |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::_processBuffer() {
  ///_debugStream->println(F("LaT:pb: enter"));
  char* txGtr = (char*)_txBuffer;                //Pointer to where in the buffer we're up to
  uint8_t length = 0;                            //Number of bytes to send from buffer
  uint8_t buffLength = _txPutter * _PACKET_SIZE; //Number of bytes in _txBuffer
  int8_t result = -1;

  while (txGtr < (char*)_txBuffer + buffLength) {
    getDataRate();                               //Update data rate public member
    switch (dataRate) {
      case '4':
        length = floor(_DR4_PAYLOAD_USAGE/_PACKET_SIZE) * _PACKET_SIZE;
        break;
      case '3':
        length = floor(_DR3_PAYLOAD_USAGE/_PACKET_SIZE) * _PACKET_SIZE;
        break;
      case '2':
        length = floor(_DR2_PAYLOAD_USAGE/_PACKET_SIZE) * _PACKET_SIZE;
        break;
      case '1':
        length = floor(_DR1_PAYLOAD_USAGE/_PACKET_SIZE) * _PACKET_SIZE;
        break;
      case '0':
      default:
        length = floor(_DR0_PAYLOAD_USAGE/_PACKET_SIZE) * _PACKET_SIZE;
    }

    //Ensure the program doesn't read past the allocated memory
    if (txGtr + length > (char*)_txBuffer + buffLength) {
      length = (char*)_txBuffer + buffLength - txGtr;
    }

    //_debugStream->print(F("LaT:pb: DR: "));
    //_debugStream->print(dataRate);
    //_debugStream->print(F(", length: "));
    //_debugStream->println(String(length));
    result = send(txGtr,length,10000);

    txGtr += length;
  }

  return(0);
}

/*----------------------------------------------------------------------------------|
| Sets default settings recomended by Campbell Scientific Australia                 |
|                                                                                   |
| Some of these default settings may be required for successful communication with  |
| the Campbell Scientific Australia LoRaWAN server.                                 |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::setDefaults() {
  int8_t result = -1;

  const char key[] PROGMEM = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01";
  const char id[] PROGMEM = "00:00:aa:00:00:00:00:01";

  if (setFrequencySubBand('1') == 0) {
    result = 0;
  }
  if (setPublicNetwork('1') == 0) {
    result = 0;
  }
  if (setNetworkID((char*)id) == 0) {
    result = 0;
  }
  if (setNetworkKey((char*) key) == 0) {
    result = 0;
  }
  if (setDataRate('2') == 0) {
    result = 0;
  }
  if (setAdaptiveDataRate('1') == 0) {
    result = 0;
  }

  commitSettings();

  return(result);
}

/*----------------------------------------------------------------------------------|
| Sets the frequency sub band                                                       |
|                                                                                   |
| AT+FSB ?                                                                          |
| AT+FSB: (0-8)                                                                     |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::setFrequencySubBand(char fsb) {
  int8_t ansCode;

  sprintf_P(_command,(char*)F("AT+FSB "));
  _command[7] = fsb;
  _command[8] = '\0';

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000);
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
int8_t LoRaAT::getFrequencySubBand() {
  int8_t ansCode;
  char* r;

  sprintf_P(_command,(char*)F("AT+FSB?"));

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000, &r);

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
int8_t LoRaAT::setPublicNetwork(char pn) {
  int8_t ansCode;

  sprintf_P(_command,(char*)F("AT+PN "));
  if (pn != '0') {
    pn = '1';
  }
  _command[6] = pn;
  _command[7] = '\0';

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000);

  if (ansCode == 1) {
    publicNetwork = pn;
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the public network                                                           |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::getPublicNetwork() {
  int8_t ansCode;
  char* r;

  sprintf_P(_command,(char*)F("AT+PN?"));

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000, &r);

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
int8_t LoRaAT::setNetworkID(char* id) {
  int8_t ansCode;

  sprintf_P(_command,(char*)F("AT+NI 0,"));
  strcat(_command,id);                           //Append ID to command

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000);

  if (ansCode == 1) {
    strncpy(networkId,id,sizeof(networkId)-1);
    networkId[(sizeof(networkId)-1)] = '\0';
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the network ID                                                               |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::getNetworkID() {
  int8_t ansCode;
  char* r;

  sprintf_P(_command,(char*)F("AT+NI?"));

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000, &r);

  if (ansCode == 1) {
    strncpy(networkId,r,(sizeof(networkId)-1));
    networkId[(sizeof(networkId)-1)] = '\0';
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
int8_t LoRaAT::setNetworkKey(char* key) {
  int8_t ansCode;

  sprintf_P(_command,(char*)F("AT+NK 0,"));
  strcat(_command,key);                          //Append key to command

  ansCode = _sendCommand(_command,(char*)&answer1,(char*)&answerX,(char*)&answerX,(char*)&answerX,10000);

  if (ansCode == 1) {
    strncpy(networkKey,key,sizeof(networkKey)-1);
    networkKey[(sizeof(networkKey)-1)] = '\0';
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the network key                                                              |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::getNetworkKey() {
  int8_t ansCode;
  char* r;

  sprintf_P(_command,(char*)F("AT+NK?"));

  ansCode = _sendCommand(_command,(char*)&answer1,(char*)&answerX,(char*)&answerX,(char*)&answerX,10000, &r);

  if (ansCode == 1) {
    strncpy(networkKey,r,(sizeof(networkKey)-1));
    networkKey[(sizeof(networkKey)-1)] = '\0';
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
int8_t LoRaAT::setDataRate(char txdr) {
  int8_t ansCode;

  sprintf_P(_command,(char*)F("AT+TXDR "));
  _command[8] = txdr;
  _command[9] = '\0';

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000);

  if (ansCode == 1) {
    dataRate = txdr;
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the data rate                                                                |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::getDataRate() {
  int8_t ansCode;
  char* r;

  sprintf_P(_command,(char*)F("AT+TXDR?"));

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000, &r);

  if (ansCode == 1) {
    dataRate = r[2];
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Turns adaptive data rate off/on.                                                  |
|                                                                                   |
| AT+ADR ?                                                                          |
| AT+ADR: (0,1)                                                                     |
|  0 - off                                                                          |
|  1 - on                                                                           |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::setAdaptiveDataRate(char adr) {
  int8_t ansCode;

  sprintf_P(_command,(char*)F("AT+ADR "));
  _command[7] = adr;
  _command[8] = '\0';

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000);

  if (ansCode == 1) {
    adaptiveDataRate = adr;
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the state of the adaptive data rate.                                         |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::getAdaptiveDataRate() {
  int8_t ansCode;

  char* r;

  sprintf_P(_command,(char*)F("AT+ADR?"));

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000, &r);

  if (ansCode == 1) {
    adaptiveDataRate = r[0];
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the device ID                                                                |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::getDeviceId() {
  int8_t ansCode;
  char* r;

  sprintf_P(_command,(char*)F("AT+DI?"));

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000, &r);

  if (ansCode == 1) {
    strncpy(deviceId,r,(sizeof(deviceId)-1));
    deviceId[(sizeof(deviceId)-1)] = '\0';
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the network address                                                          |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::getNetworkAddress() {
  int8_t ansCode;
  char* r;

  sprintf_P(_command,(char*)F("AT+NA?"));

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000, &r);

  if (ansCode == 1) {
    strncpy(networkAddress,r,(sizeof(networkAddress)-1));
    networkAddress[(sizeof(networkAddress)-1)] = '\0';
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the network session key                                                      |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::getNetworkSessionKey() {
  int8_t ansCode;
  char* r;

  sprintf_P(_command,(char*)F("AT+NSK?"));

  ansCode = _sendCommand(_command,(char*)&answer1,(char*)&answerX,(char*)&answerX,(char*)&answerX,10000, &r);

  if (ansCode == 1) {
    strncpy(networkSessionKey,r,(sizeof(networkSessionKey)-1));
    networkSessionKey[(sizeof(networkSessionKey)-1)] = '\0';
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the data session key                                                         |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::getDataSessionKey() {
  int8_t ansCode;
  char* r;

  sprintf_P(_command,(char*)F("AT+DSK?"));

  ansCode = _sendCommand(_command,(char*)&answer1,(char*)&answerX,(char*)&answerX,(char*)&answerX,10000, &r);

  if (ansCode == 1) {
    strncpy(dataSessionKey,r,(sizeof(dataSessionKey)-1));
    dataSessionKey[(sizeof(dataSessionKey)-1)] = '\0';
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Saves the current session information in the mDot.                                |
-----------------------------------------------------------------------------------*/
/*
int8_t LoRaAT::saveLoraSession() {
  int8_t ansCode;
  char ans1[] PROGMEM = "OK";

  sprintf_P(_command,(char*)F("AT+SS"));

  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000);
  if (ansCode < 0 ) {
    return(-1);
  }
  ansCode = getNetworkSessionKey();
  if (ansCode < 0 ) {
    return(-1);
  }
  return(getDataSessionKey());
}
*/
/*----------------------------------------------------------------------------------|
| Saves the current session information in the mDot.                                |
-----------------------------------------------------------------------------------*/
/*
int8_t LoRaAT::restoreLoraSession() {
  int8_t ansCode;
  char ans1[] PROGMEM = "OK";

  sprintf_P(_command,(char*)F("AT+RS"));

  ansCode = _sendCommand(_command,ans1,NULL,NULL,NULL,10000);
  if (ansCode < 0 ) {
    return(-1);
  }
  ansCode = getNetworkSessionKey();
  if (ansCode < 0 ) {
    return(-1);
  }
  ansCode = getDataSessionKey();

  if (ansCode == 1) {
    return (0);
  }

  return(-1);
}
*/
/*----------------------------------------------------------------------------------|
| Writes the current settings of the mDot to it's memory                            |
-----------------------------------------------------------------------------------*/
int8_t LoRaAT::commitSettings() {
  int8_t ansCode;

  sprintf_P(_command,(char*)F("AT&W"));

  ansCode = _sendCommand(_command,(char*)&answer1,NULL,NULL,NULL,10000);

  if (ansCode == 1) {
    return (0);
  }

  return(-1);
}
