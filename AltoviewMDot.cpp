/*
  File: AltoviewMDot.cpp

  Version: v0.2.1

  Brief: Arduino library for controlling Multitech mDot LoRa modules using
         AT commands.

  Copyright: This library is published under GNU AGPLv3 license.
             http://choosealicense.com/licenses/agpl-3.0/

  Author: Campbell Scientific Australia Pty Ltd
*/

/************************************************************************************
 *                                   INCLUDES                                       *
 ***********************************************************************************/
#include "AltoviewMDot.h"

#define DEBUG

const char command_00[]  PROGMEM = "AT+FSB ";
const char command_01[]  PROGMEM = "AT+PN ";
const char command_02[]  PROGMEM = "AT+NI 0,";
const char command_03[]  PROGMEM = "AT+NK 0,";
const char command_04[]  PROGMEM = ""; //vacant
const char command_05[]  PROGMEM = "AT+JOIN";
const char command_06[]  PROGMEM = "AT+SEND ";
const char command_07[]  PROGMEM = "AT+SNR";
const char command_08[]  PROGMEM = "AT+RSSI";
const char command_09[]  PROGMEM = "AT+FSB?";
const char command_10[]  PROGMEM = "AT+PN?";
const char command_11[]  PROGMEM = "AT+NI?";
const char command_12[]  PROGMEM = "AT+NK?";
const char command_13[]  PROGMEM = "AT+TXDR";
const char command_14[]  PROGMEM = "AT+TXDR=DR";
const char command_15[]  PROGMEM = "AT+TXDR?";
const char command_16[]  PROGMEM = "AT+ADR ";
const char command_17[]  PROGMEM = "AT+ADR?";
const char command_18[]  PROGMEM = "AT+DI?";
const char command_19[]  PROGMEM = "AT+NA?";
const char command_20[]  PROGMEM = "AT+NSK?";
const char command_21[]  PROGMEM = "AT+DSK?";
const char command_22[]  PROGMEM = "AT&W";

const char* const table_LoRaWAN_COMMANDS[] PROGMEM=     
{   
  command_00, 
  command_01,
  command_02,
  command_03,
  command_04,
  command_05,
  command_06,
  command_07,
  command_08,
  command_09,
  command_10,
  command_11,
  command_12,
  command_13,
  command_14,
  command_15,
  command_16,
  command_17,
  command_18,
  command_19,
  command_20,
  command_21,
  command_22
};

const char answer_00[] PROGMEM = "OK";
const char answer_01[] PROGMEM = "BUG";

const char* const table_LoRaWAN_ANSWERS[] PROGMEM=    
{   
  answer_00, 
  answer_01
};


const char key_NetworkKey[]   PROGMEM = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01";
const char key_NetworkId[]    PROGMEM = "00:00:aa:00:00:00:00:01";

const char* const table_LoRaWAN_KEYS[] PROGMEM=     
{   
  key_NetworkId,
  key_NetworkKey
};

/************************************************************************************
 *                               PUBLIC FUNCTIONS                                   *
 ***********************************************************************************/

	/*----------------------------------------------------------------------------------|
	| CONSTRUCTOR: Creates class object using a specified serial port, and passing it a |
	| specified debug stream. This stream can be used to pass debug info to a serial    |
	| port, either hardware serial or software serial.                                  |
	-----------------------------------------------------------------------------------*/
	AltoviewMDot::AltoviewMDot(AltSoftSerial* mdot_serial, HardwareSerial* debug_serial) {
	  //TODO: Input checking, what range of values to accept, how to handle invalid input
	  //_u8SerialPort = mdot_serial;       //legacy 
		_debug_serial = debug_serial;
		_mdot_serial = mdot_serial; 
	}


/*----------------------------------------------------------------------------------|
| Initialize class object.                                                          |
|                                                                                   |
| Sets up the serial port specified in instantiation using default 38400 baud rate. |
| Then configures the mDot with some default settings.                              |
|                                                                                   |
| Call once class has been instantiated, typically within setup().                  |
-----------------------------------------------------------------------------------*/
void AltoviewMDot::begin() {
  setDefaults();
}

/*----------------------------------------------------------------------------------|
| The send command method. Takes a null terminated command, some responses, and a   |
| timeout.                                                                          |
|                                                                                   |
| The command is sent to the mDot and waits for a response. Once a recognised       |
| response is received it returns the corresponding integer. If no recognised       |
| response is received within the timeout period a -1 is returned.                  |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::_sendCommand(char* command, char* answer1, char* ans2, uint16_t timeout) {
  return _sendCommand(command, answer1, ans2, timeout, NULL);
}

// char** AltoviewMDot::compileResp(char* ans)
// {
//     if (answer1 != NULL && 
//         strstr(_response, answer1) != '\0' && 
//         (resp != NULL) {
//         *resp = strstr(_response,command);
//         *resp += strlen(command);
//         *resp += sizeof(TERMINATOR);
//       }
// }
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
|                                                                                   |
| *Note,                                                                            |
| We have also reduced the number of answers accepted by the function from 4 to 2 as|
| we were facing a GCC bug in-case of checking 4 answers.                           |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::_sendCommand(char* command, char* ans1, char* ans2, uint16_t timeout, char** resp)  {
  ///_debug_serial->println(F("LaT:sc: enter"));
  static const char TERMINATOR[3] = {'\r','\n','\0'};

  uint32_t maxEndTime = 0;

  //flush receive buffer before transmitting request
  delay(20);                                     				//Undesirable delay, if we read/write too quick to the mDot,  timing issues arise.
  while (_mdot_serial->read() != -1);
  delay(20);                                     				//Undesirable delay, if we read/write too quick to the mDot,  timing issues arise.

  memset(_response,0x00,_MAX_MDOT_RESPONSE);					//Blank string
  _length = 0;

#ifdef DEBUG
  //Send command
  _debug_serial->print(F("LaT:sc: "));
  _debug_serial->print(command);
  _debug_serial->print(TERMINATOR);
#endif

  // _mdot_serial->flush();
  
  _mdot_serial->print(command);
  _mdot_serial->print(TERMINATOR);

  // _mdot_serial->flush() ;

  maxEndTime = millis() + timeout;								//Set timeout time

  //While something is available get it
  ///_debug_serial->println(F("LaT:sc: Loop collecting response"));
  *resp = NULL;

  int availableCount = 0;
  int tempRet = -1;
  do {
    availableCount = _mdot_serial->available();
    if (availableCount != 0) {							//available() is a method of the serial class showing number of available bytes to read. 
      // _debug_serial->print("[");
      // _debug_serial->print(availableCount);
      // _debug_serial->print("]");
      if (_length < (_MAX_MDOT_RESPONSE - 2)) {
        _response[_length++] = _mdot_serial->read();
        // _debug_serial->write(_response[_length - 1]);
        // _debug_serial->println();
        _response[_length] = '\0';               				//Ensure response buffer is null terminated
      }

      if (ans1 != NULL && strstr(_response, ans1) != '\0') {
        tempRet = 1;
        break;
      }

      if (ans2 != NULL && strstr(_response, ans2) != '\0') {
        tempRet = 2;
        break;
      }

    }
  } while (millis() <= maxEndTime);

  if (resp != NULL && tempRet != -1) {
    //_debug_serial->print(F("LaT:sc: "));
    //_debug_serial->println(_response);
    *resp = strstr(_response,command);
    *resp += strlen(command);
    *resp += sizeof(TERMINATOR);
  }

  if (tempRet == -1)
  {
    _debug_serial->println(F("LaT:sc: Timed out"));
  }
  
  return (tempRet);
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
int8_t AltoviewMDot::join() {
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
int8_t AltoviewMDot::join(uint16_t timeout) {
  ///_debug_serial->println(F("LaT:j : enter"));
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));

  // sprintf_P(_command,(char*)F("AT+JOIN"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[5]))); 
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 

  ansCode = _sendCommand(_command, answer1,NULL,timeout);
  if (ansCode < 0 ) {
    return(-1);
  }

  return(0);
}

/*----------------------------------------------------------------------------------|
| Leave a LoRa(WAN?) network                                                        |
-----------------------------------------------------------------------------------*/
void AltoviewMDot::leave() {
  ///_debug_serial->println(F("LaT:l: not implemented"));
}

/*----------------------------------------------------------------------------------|
| Send method, takes a null terminated char array and sends that over the LoRaWAN   |
| network. Using a default timeout of 10,000ms (10sec).                             |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::send(char* message) {
  ///_debug_serial->println(F("LaT:s : enter, w/ default timeout"));
  return(send(message,10000));
}

/*----------------------------------------------------------------------------------|
| Send method, takes a null terminated char array and sends that over the LoRaWAN   |
| network. Using a specified timeout.                                               |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::send(char* message, uint16_t timeout) {
  ///_debug_serial->println(F("LaT:s : enter"));
  uint8_t l;

  l = sizeof(_command);                     //Will only send char array up to null, or l (max size of _command buffer)

  return(send(message,l,10000));
}

/*----------------------------------------------------------------------------------|
| Send method, takes a char array and length and sends that over the LoRaWAN        |
| network. Using a specified timeout.                                               |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::send(char* message, uint8_t length, uint16_t timeout) {
  ///_debug_serial->println(F("LaT:s : enter"));
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));

  // sprintf_P(_command,(char*)F("AT+SEND "));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[6])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 
  strncat(_command,message,length);              //Append message to command

  ansCode = _sendCommand(_command, answer1,NULL,timeout);

  if (ansCode == 1) {
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the RSSI and SNR using their AT commands. Saves them to public variables.    |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::ping() {
	int8_t ansCode1;
	int8_t ansCode2;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
	char* r;

  	ansCode1 = getSnr(); 
  	ansCode2 = getRssi(); 

  if (ansCode1 == 1 && ansCode2 == 1) {
      return (0);
    }
  return(-1);
}

int8_t AltoviewMDot::getSnr() {
  //_debug_serial->println(F("LaT:snr:"));
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char* r;
  char temp[5];									//variable to store potential length snr of latest packet 

  // sprintf_P(_command,(char*)F("AT+SNR"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[7])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 
  ansCode = _sendCommand(_command, answer1,NULL,10000, &r);

  if (ansCode == 1) {
    //strncpy(snr,r,(sizeof(snr)-1)); 
  	sprintf(temp, "%.5s",r);					//get the first 5 values from the response array				
  	snr = (float)atof(temp);					//use atoi to cast char array to uint16 and strip ' ' or ','
    return (0);
  }

  return(-1);
}

int8_t AltoviewMDot::getRssi() {
  //_debug_serial->println(F("LaT:rssi:"));
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char* r;
  char temp[4];									//variable to store potential length rssi of latest packet 

  // sprintf_P(_command,(char*)F("AT+RSSI"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[8])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 

  ansCode = _sendCommand(_command, answer1,NULL,10000, &r);

  if (ansCode == 1) {
    //strncpy(snr,r,(sizeof(snr)-1)); 
  	sprintf(temp, "%.4s",r);					//get the first 4 values from the response array				
  	rssi = (int16_t)atof(temp);					//use atoi to cast char array to uint16 and strip ' ' or ','
    return (0);
  }

  return(-1);
}



/*----------------------------------------------------------------------------------|
| Receives a string in the format key:value,key:value,...                           |
|                                                                                   |
| 1. The csv is translated to a JSON.                                               |
| 2. The json is fragmented to the _txBuffer .                                      |
| 3. The _txBuffer is processed (sent).                                             |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::sendPairs(String pairs) {
  return(sendPairs(&pairs));
}

/*----------------------------------------------------------------------------------|
| Receives a string in the format key:value,key:value,...                           |
|                                                                                   |
| 1. The csv is translated to a JSON.                                               |
| 2. The json is fragmented to the _txBuffer .                                      |
| 3. The _txBuffer is processed (sent).                                             |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::sendPairs(String* pairs) {
  ///_debug_serial->println(F("LaT:sp: enter"));
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
int8_t AltoviewMDot::sendPairs(char* pairs) {

#ifdef DEBUG
  ///_debug_serial->println(F("LaT:sp: enter"));
  _debug_serial->print(F("LaT:sp: "));
  _debug_serial->println(pairs);
#endif
  //String json;
  char json[_MAX_PAIRS_SIZE];
  memcpy(json,0x00,_MAX_PAIRS_SIZE);

  //TODO: Check the string is actually pairs

  ///_debug_serial->println(F("LaT:sp: convert to JSON"));
  _pairsToJSON(json, _MAX_PAIRS_SIZE, pairs);

#ifdef DEBUG
  _debug_serial->print(F("LaT:sp: "));
  _debug_serial->println(json);
  ///_debug_serial->println(F("LaT:sp: fragment JSON to buffer"));
#endif

  _createFragmentBuffer(json);
  ///_debug_serial->println(F("LaT:sp: process buffer"));

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
void AltoviewMDot::_pairsToJSON(char* json, uint8_t jsonLength, char* pairs) {
  ///_debug_serial->println(F("LaT:pj: enter"));
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

  ///_debug_serial->println(F("LaT:pj: exit"));
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
void AltoviewMDot::_createFragmentBuffer(char* message) {
  ///_debug_serial->println(F("LaT:fb: enter"));

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
    ///_debug_serial->println(F("LaT:fb: max frags exceeded"));
    numFragments = _MAX_FRAGMENTS;
  }

  ///_debug_serial->print(F("LaT:fb: create "));
  ///_debug_serial->print(numFragments);
  ///_debug_serial->println(F(" fragments"));
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

    ///_debug_serial->print(F("LaT:fb: create fragment "));
    ///_debug_serial->println(_txPutter);
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
    ///_debug_serial->print(F("LaT:fb: "));
    ///_debug_serial->println(_txBuffer[_txPutter]);
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
int8_t AltoviewMDot::_processBuffer() {
  ///_debug_serial->println(F("LaT:pb: enter"));
  char* txGtr = (char*)_txBuffer;                //Pointer to where in the buffer we're up to
  uint8_t length = 0;                            //Number of bytes to send from buffer
  uint8_t buffLength = _txPutter * _PACKET_SIZE; //Number of bytes in _txBuffer
  int8_t result = -1;

  while (txGtr < (char*)_txBuffer + buffLength) {
    getDataRate();                               //Update data rate public member
    switch (dataRate) {
      case 4:
        length = floor(_DR4_PAYLOAD_USAGE/_PACKET_SIZE) * _PACKET_SIZE;
        break;
      case 3:
        length = floor(_DR3_PAYLOAD_USAGE/_PACKET_SIZE) * _PACKET_SIZE;
        break;
      case 2:
        length = floor(_DR2_PAYLOAD_USAGE/_PACKET_SIZE) * _PACKET_SIZE;
        break;
      case 1:
        length = floor(_DR1_PAYLOAD_USAGE/_PACKET_SIZE) * _PACKET_SIZE;
        break;
      case 0:
      default:
        length = floor(_DR0_PAYLOAD_USAGE/_PACKET_SIZE) * _PACKET_SIZE;
    }

    //Ensure the program doesn't read past the allocated memory
    if (txGtr + length > (char*)_txBuffer + buffLength) {
      length = (char*)_txBuffer + buffLength - txGtr;
    }

    //_debug_serial->print(F("LaT:pb: DR: "));
    //_debug_serial->print(dataRate);
    //_debug_serial->print(F(", length: "));
    //_debug_serial->println(String(length));
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
int8_t AltoviewMDot::setDefaults() {
  int8_t result = -1;
  char id[sizeof(key_NetworkId)];
  char key[sizeof(key_NetworkKey)];
  sprintf_P(id, (char*)pgm_read_word(&(table_LoRaWAN_KEYS[0])));
  sprintf_P(key,(char*)pgm_read_word(&(table_LoRaWAN_KEYS[1])));

  if (setFrequencySubBand('1') == 0) {
    result = 0;
  }
  if (setPublicNetwork('1') == 0) {
    result = 0;
  }
  if (setNetworkID(id) == 0) {
    result = 0;
  }
  if (setNetworkKey(key) == 0) {
    result = 0;
  }
  if (setDataRate((uint8_t)2) == 0) {
    result = 0;
  }
  if (setAdaptiveDataRate('0') == 0) {
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
int8_t AltoviewMDot::setFrequencySubBand(char fsb) {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));


  // sprintf_P(_command,(char*)F("AT+FSB "));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[0])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 


  _command[7] = fsb;
  _command[8] = '\0';

  ansCode = _sendCommand(_command,answer1,NULL,10000);
  ///_debug_serial->println(_response);

  if (ansCode == 1) {
    frequencySubBand = fsb;
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the frequency sub band                                                       |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::getFrequencySubBand() {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char* r;

  // sprintf_P(_command,(char*)F("AT+FSB?"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[9])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 

  ansCode = _sendCommand(_command, answer1,NULL,10000, &r);

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
int8_t AltoviewMDot::setPublicNetwork(char pn) {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));

  // sprintf_P(_command,(char*)F("AT+PN "));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[1])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 
  if (pn != '0') {
    pn = '1';
  }
  _command[6] = pn;
  _command[7] = '\0';

  ansCode = _sendCommand(_command, answer1,NULL,10000);

  if (ansCode == 1) {
    publicNetwork = pn;
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the public network                                                           |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::getPublicNetwork() {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char* r;

  // sprintf_P(_command,(char*)F("AT+PN?"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[10])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 

  ansCode = _sendCommand(_command, answer1,NULL,10000, &r);

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
int8_t AltoviewMDot::setNetworkID(char* id) {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));

  // sprintf_P(_command,(char*)F("AT+NI 0,"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[2])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 
  strcat(_command,id);                           //Append ID to command

  ansCode = _sendCommand(_command, answer1,NULL,10000);

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
int8_t AltoviewMDot::getNetworkID() {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char* r;

  // sprintf_P(_command,(char*)F("AT+NI?"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[11])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 

  ansCode = _sendCommand(_command, answer1,NULL,10000, &r);

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
int8_t AltoviewMDot::setNetworkKey(char* key) {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char answerX[5];
  memset(answerX,0x00,sizeof(answerX));
  memset(answerX,0x00,sizeof(answerX));

  // TODO, remove
  // char keyz[46];
  // sprintf_P(keyz,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[4])));

  // sprintf_P(_command,(char*)F("AT+NK 0,"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[3])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 
  sprintf_P(answerX,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[1]))); 
  strcat(_command,key);                          //Append key to command
  // strcat(_command,keyz);                          //Append key to command

  // sprintf_P(keyz,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[5])));
  // strcat(_command,keyz);                          //Append key to command

  ansCode = _sendCommand(_command, answer1, answerX,10000);

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
int8_t AltoviewMDot::getNetworkKey() {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char answerX[5];
  memset(answerX,0x00,sizeof(answerX));
  char* r;

  // sprintf_P(_command,(char*)F("AT+NK?"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[12])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 
  sprintf_P(answerX,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[1]))); 

  ansCode = _sendCommand(_command, answer1, answerX,10000, &r);

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
|																					                                          |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::setDataRate(char txdr) {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));

  // sprintf_P(_command,(char*)F("AT+TXDR"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[13])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 
  _command[8] = txdr;
  _command[9] = '\0';

  ansCode = _sendCommand(_command, answer1,NULL,10000);

  if (ansCode == 1) {
    dataRate = (uint8_t)txdr;
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------
| Sets the data rate                                                                
|                                                                                   
| AT+TXDR ?                                                                         
| AT+TXDR: (DR0-DR4|DR8-DR13) 														                          
|																					                                          
| Eg:                                                                               
|         |----------|----------|----------|                              
|					| DataRate | Config   | Bit rate |								
|					|----------|----------|----------|								
|					| 	DR0    |SF10BW125 |   980    |								
|					| 	DR1    |SF09BW125 |  1760    |								
|					| 	DR2    |SF08BW125 |  3125    |								
|					| 	DR3    |SF07BW125 |  5470    |								
|					| 	DR4    |SF08BW500 | 12500    |								
|					| DR5:DR7  | Not Used |          |								
|					| 	DR8    |SF12BW500 |   980    |								
|					| 	DR9    |SF11BW500 |  1760    |							
|					|   DR10   |SF10BW500 |  3900    |						
|					| 	DR11   |SF09BW500 |  7000    |								
|					| 	DR12   |SF08BW500 | 12500    |								
|					| 	DR13   |SF07BW500 | 21900    |								
|					| DR14:DR15| Not Used |          |								
|         |----------|----------|----------|                              
|																					
|																					
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::setDataRate(uint8_t txdr) {
  int8_t ansCode;
  char temp[2]; 								//temp char array to store txdr range of 0-13
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));

  // sprintf_P(_command,(char*)F("AT+TXDR=DR")); 	//_command = {'A','T','+','T','X','D','R','=','D','R','X','Y','\0'}
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[14])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 
  sprintf(temp, "%u",txdr);						//convert txdr to temp char array

  strcat(_command,temp);						//concatenate _command with temp
  _command[12] = '\0';

  ansCode = _sendCommand(_command, answer1,NULL,10000);

  if (ansCode == 1) {
    dataRate = txdr;
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the data rate                                                                |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::getDataRate() {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char* r;
  char temp[2];

  // sprintf_P(_command,(char*)F("AT+TXDR?"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[15])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 

  ansCode = _sendCommand(_command, answer1,NULL,10000, &r);

  if (ansCode == 1) {
  	temp[0] = r[2];						//get first char from response
  	if (r[3] != ' '){					//if next character is not white space --> response can be ("DRX - SFXXBWXXX" | "DRXX - SFXXBWXXX")
  		temp[1] = r[3];					//store third character of response to temp char array 
  	}
    dataRate = (uint8_t)atoi(temp);		//convert char* to uint8_t 
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
int8_t AltoviewMDot::setAdaptiveDataRate(char adr) {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));

  // sprintf_P(_command,(char*)F("AT+ADR "));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[16])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 
  _command[7] = adr;
  _command[8] = '\0';

  ansCode = _sendCommand(_command, answer1,NULL,10000);

  if (ansCode == 1) {
    adaptiveDataRate = adr;
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the state of the adaptive data rate.                                         |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::getAdaptiveDataRate() {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char* r;

  // sprintf_P(_command,(char*)F("AT+ADR?"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[17])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 

  ansCode = _sendCommand(_command, answer1,NULL,10000, &r);

  if (ansCode == 1) {
    adaptiveDataRate = r[0];
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Gets the device ID                                                                |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::getDeviceId() {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char* r;

  // sprintf_P(_command,(char*)F("AT+DI?"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[18])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 

  ansCode = _sendCommand(_command, answer1,NULL,10000, &r);

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
int8_t AltoviewMDot::getNetworkAddress() {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char* r;

  // sprintf_P(_command,(char*)F("AT+NA?"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[19])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 

  ansCode = _sendCommand(_command, answer1,NULL,10000, &r);

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
int8_t AltoviewMDot::getNetworkSessionKey() {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char answerX[5];
  memset(answerX,0x00,sizeof(answerX));
  char* r;

  // sprintf_P(_command,(char*)F("AT+NSK?"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[20])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 
  sprintf_P(answerX,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[1]))); 

  ansCode = _sendCommand(_command, answer1, answerX,10000, &r);

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
int8_t AltoviewMDot::getDataSessionKey() {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));
  char answerX[5];
  memset(answerX,0x00,sizeof(answerX));
  char* r;

  // sprintf_P(_command,(char*)F("AT+DSK?"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[21])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 
  sprintf_P(answerX,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[1]))); 

  ansCode = _sendCommand(_command, answer1, answerX,10000, &r);

  if (ansCode == 1) {
    strncpy(dataSessionKey,r,(sizeof(dataSessionKey)-1));
    dataSessionKey[(sizeof(dataSessionKey)-1)] = '\0';
    return (0);
  }

  return(-1);
}

/*----------------------------------------------------------------------------------|
| Writes the current settings of the mDot to it's memory                            |
-----------------------------------------------------------------------------------*/
int8_t AltoviewMDot::commitSettings() {
  int8_t ansCode;
  char answer1[5];
  memset(_command,0x00,sizeof(_command));
  memset(answer1,0x00,sizeof(answer1));

  // sprintf_P(_command,(char*)F("AT&W"));
  sprintf_P(_command,(char*)pgm_read_word(&(table_LoRaWAN_COMMANDS[22])));
  sprintf_P(answer1,(char*)pgm_read_word(&(table_LoRaWAN_ANSWERS[0]))); 

  ansCode = _sendCommand(_command, answer1,NULL,10000);

  if (ansCode == 1) {
    return (0);
  }

  return(-1);
}