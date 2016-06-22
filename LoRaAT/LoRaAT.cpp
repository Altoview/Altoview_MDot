/*
  File: LoRaAT.cpp

  Version: v0.0.1  

  Breif: Arduino library for controlling Multitech mDot LoRa modules using 
         AT commands.

  Copyright: This library is published under GNU AGPLv3 license.
             http://choosealicense.com/licenses/agpl-3.0/

  Auther: Campbell Scientific Australia Pty Ltd
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
  HardwareSerial* MBSerial = &Serial; //< Pointer to Serial class object
#elif defined(ARDUINO_ARCH_SAM)
  UARTClass* MBSerial = &Serial;      //< Pointer to Serial class object
#else
  #error "This library only supports boards with an AVR or SAM processor."
#endif


/************************************************************************************
 *                               PUBLIC FUNCTIONS                                   *
 ***********************************************************************************/

/*
 * CONSTRUCTOR: 
 * Creates class object using a default serial port 0
 */
  LoRaAT::LoRaAT() {
    _u8SerialPort = 0;
  }

/*
 * CONSTRUCTOR: 
 * Creates class object using a specified serial port
 */
  LoRaAT::LoRaAT(uint8_t u8SerialPort) {
  //TODO: Input checking, what range of values to accept, how to handle invalid input
    _u8SerialPort = u8SerialPort;
  }

/*
 * Initialize class object.
 * 
 * Sets up the serial port using default 19200 baud rate.
 * Call once class has been instantiated, typically within setup().
 */
  void LoRaAT::begin(void) {
  //TODO: Input checking??
    begin(19200);
  }

/*
 * Initialize class object.
 * 
 * Sets up the serial port using specified baud rate.
 * Call once class has been instantiated, typically within setup().

  TODO: Think about what we should set here, what should be defaults. Datarates,
  adaptive data rates? Other things?
*/
  void LoRaAT::begin(uint32_t u32BaudRate) {
    switch(_u8SerialPort) {
    #if defined(UBRR1H)
      case 1:
      MBSerial = &Serial1;
      break;
    #endif
      
    #if defined(UBRR2H)
      case 2:
      MBSerial = &Serial2;
      break;
    #endif
      
    #if defined(UBRR3H)
      case 3:
      MBSerial = &Serial3;
      break;
    #endif
      
      case 0:
      default:
      MBSerial = &Serial;
      break;
    }

    MBSerial->begin(u32BaudRate);
  }

/*
  Join a LoRa(WAN?) network
  
  TODO: Review returns, can we return something more meaningfull?
  Currently, zero is a success, negative integers are generic errors, and positive
  integers are a specific failure.
    0 - success
    1 - mdot responds with failure to join network response
   -1 - mdot responds with "ERROR"
   -2 - Timeout error
  
  //TODO: parameters, saying something about network we want to join?
  
  uses a default of 10,000ms (10sec) timeout
*/
  int LoRaAT::join() {
    return(join(10000));
  }

/*
  Join a LoRa(WAN?) network
  
  TODO: Review returns, can we return something more meaningfull?
  Currently, zero is a success, negative integers are generic errors, and positive
  integers are a specific failure.
    0 - success
    1 - mdot responds with failure to join network response
   -1 - Timeout error
   -2 - mdot responds with "ERROR"
  
  //TODO: parameters, saying something about network we want to join?
  
  takes the parameter timeout, which is the number of milliseconds you want it
  to wait for a response.
*/
  int LoRaAT::join(unsigned int timeout) {

  SoftwareSerial debugSerial(10, 11);     // RX, TX
  debugSerial.begin(38400);   //Debug output. Listen on this ports for debugging info
  debugSerial.println("DEBUG: Joining");

  const int LOOP_DELAY = 250;			//Millisecond delay between serial attempts
  unsigned long timeoutCounter = 0;		//We don't wait forever for a response
  // String _recievedString;				//String returned by device
  char _recievedString[100];       //String returned by device
  int available;

  //flush receive buffer before transmitting request
  while (MBSerial->read() != -1);
  
  //Send join request
  MBSerial->println("AT+JOIN");
  
  //Loop reading from serial buffer until we get either a recognisable error, or
  //success
  timeoutCounter = 0;
  delay(10000);

  while(timeoutCounter < timeout) {
    // Blank string
    for (int i = 0; i < 100; i++) 
    {
      _recievedString[i] = '\0';
    }
    available = MBSerial->available();
    if (available) {
      // _recievedString = MBSerial->readString();

      if(MBSerial->readBytesUntil(NULL, _recievedString, available)) 
      {
        debugSerial.println(_recievedString);
        debugSerial.println("DEBUG: Joined");
        if (strstr(_recievedString, "OK") != NULL)
          return (0);
      }
      //     if (_recievedString.indexOf("Failed to join network") >= 0) {
      //       return(1);
      //     } else if (_recievedString.indexOf("ERROR") >= 0) {
      //       return(-2);
      //     } else if (_recievedString.indexOf("Successfully joined network") >= 0) {
      //       return(0);
      //     } else {
    		// //Unknown response??
      //     }
    }
    timeoutCounter += LOOP_DELAY;
    delay(LOOP_DELAY);
  }
  
  return(-1);
}

/*
  Leave a LoRa(WAN?) network
*/
  void LoRaAT::leave() {

  }

/*
  In general we send strings using the AT command, TODO: we could have overloaded functions
  to accept other things?
  
  TODO: Input checking?
    - Too long?
	  + Return error?
	  + Break it up?
	  + Send first part?
	- Too short?
	  + Is it even a problem?
	  + Pad it out?
	  + Return error?
	- Invalid characters?
	  + Is it even a problem?
	  + Send anyway?
	  + Replace with something else?
	  + Return error?
  
  uses a default of 10,000ms (10sec) timeout
*/
  int LoRaAT::send(char* message) {
    return(send(message,10000));
  }

/*
  In general we send strings using the AT command, TODO: we could have overloaded functions
  to accept other things?
  
  TODO: Input checking?
    - Too long?
	  + Return error?
	  + Break it up?
	  + Send first part?
	- Too short?
	  + Is it even a problem?
	  + Pad it out?
	  + Return error?
	- Invalid characters?
	  + Is it even a problem?
	  + Send anyway?
	  + Replace with something else?
	  + Return error?
*/
  int LoRaAT::send(char* message, unsigned int timeout) {

  SoftwareSerial debugSerial(10, 11);     // RX, TX
  debugSerial.begin(38400);   //Debug output. Listen on this ports for debugging info
  debugSerial.print("DEBUG: Sending: ");
  debugSerial.println(message);


  int LOOP_DELAY = 250;			//Millisecond delay between serial attempts
  unsigned long timeoutCounter = 0;		//We don't wait forever for a response
  char _recievedString[45];				//String returned by device
  
  //flush receive buffer before transmitting request
  while (MBSerial->read() != -1);

  int available = MBSerial->available();

  debugSerial.print("DEBUG: available =:");
  debugSerial.println(available , DEC);
  
  //Send message
  MBSerial->print("AT+SEND ");
  for (int i = 0; i < _PACKET_SIZE; i++)
  {
    MBSerial->print(message[i]);
  }
  MBSerial->println();


  //MBSerial->println("at+send " + message + "\r\n");
  delay(2000);
  
  //Loop reading from serial buffer until we get either a recognisable error, or
  //success
  //pinMode(3,OUTPUT);
  timeoutCounter = 0;

  available = MBSerial->available();
  debugSerial.print("DEBUG: available =:");
  debugSerial.println(available , DEC);

  while(timeoutCounter < timeout) {
    // Blank string
    available = MBSerial->available();
    if (available) {
      // _recievedString = MBSerial->readString();

      if(MBSerial->readBytesUntil(NULL, _recievedString, available)) 
      {
        debugSerial.println(_recievedString);
        if (strstr(_recievedString, "OK") != NULL)
          return (0);
      }
      //     if (_recievedString.indexOf("Failed to join network") >= 0) {
      //       return(1);
      //     } else if (_recievedString.indexOf("ERROR") >= 0) {
      //       return(-2);
      //     } else if (_recievedString.indexOf("Successfully joined network") >= 0) {
      //       return(0);
      //     } else {
        // //Unknown response??
      //     }
    }
    timeoutCounter += LOOP_DELAY;
    delay(LOOP_DELAY);
  }
  
  return(-1);
//  while(timeoutCounter < timeout) {
  /*
  while(available > 0) {
//  while(true) {
    debugSerial.print("<");
    debugSerial.print(timeoutCounter, DEC);
    debugSerial.print(">");
    //if (MBSerial->available() != 0) {
    _recievedString += MBSerial->readString();
      //debugSerial.println("DEBUG: Received String=("+ _recievedString +")");

      //debugSerial.print("DEBUG: Received String len=(");
      //debugSerial.print(_recievedString.length(), DEC);
      //debugSerial.println(")");
      //if (_recievedString.indexOf("at+send") < 0) {
    if (_recievedString.indexOf("ACK not received") >= 0)  {
      return(1);
    } else if (_recievedString.indexOf("ERROR") >= 0) {
      return(-2);
    } else if (_recievedString.indexOf("Data exceeds datarate max payload") >= 0) {
      return(-3);
    } else if (_recievedString.indexOf("OK") >= 0) {
      return(0);
    } else {
      debugSerial.println("else(" + _recievedString + ")");
          //return(100);
		//Unknown response??
    }
      //}
    //}
    timeoutCounter += LOOP_DELAY;
    digitalWrite(3, HIGH);
    delay(LOOP_DELAY);
    digitalWrite(3, LOW);
    delay(LOOP_DELAY);
    available = MBSerial->available();
    debugSerial.print("DEBUG: available end WHILE =:");
    debugSerial.println(available , DEC);
  }
  
  return(-1);
  */
}

/*
  I believe there maybe a ping function... I'm not sure yet what the AT command
  returns, or what we should return to the user?
*/
  uint8_t LoRaAT::ping() {

  }

// Overloading the sendPairs
 int LoRaAT::sendPairs(String pairs) 
 {
   char pairsC[_MAX_PAIRS_SIZE];
   pairs.toCharArray(pairsC, _MAX_PAIRS_SIZE);
   LoRaAT::sendPairs(pairsC);
 }

/*
 Recieves a string in the format key:value,key:value,...

 1. The csv is translated to a json,
 2. The json is added to the _txBuffer
 3. The _txBuffer is processed

 //TODO: If not recieved in that format return an error
 */
 int LoRaAT::sendPairs(char* pairs) {

  SoftwareSerial debugSerial(10, 11);     // RX, TX
  debugSerial.begin(38400);   //Debug output. Listen on this ports for debugging info
  //debugSerial.println("Send Pairs init: "+ pairs);

  //Return constants
  const byte UNKNOWN_FORMAT = 4;
  int response = 0;

//  String json;      
  char json[_MAX_PAIRS_SIZE];      
  
  //TODO: Check the string is actually pairs
  
  if (false) { //TODO: if format not recoginised
    return(UNKNOWN_FORMAT);
  }

  _pairsToJSON(json, pairs);
  debugSerial.print("DEBUG, JSON: ");
  debugSerial.println(json);
  _createFragmentBuffer(json);
  //debugSerial.println("Prior to proccessing buffer");
  response = _processBuffer();

  return(response);
}

/*
 This function will take any correctly formatted string of key:value pairs
 and return a JSON formatted String.
 */
 void LoRaAT::_pairsToJSON(char* json, char* pairs) {
  // String json = "";   //Finished JSON as a string
  // String temp = "";   //Temp storage as we look for delimters
  // String key = "";    //temp storage for the "key".. "value" is pulled straiged from temp
  
  // StaticJsonBuffer<_jsonMemeoryPool> jsonBuffer;
  // JsonObject& root = jsonBuffer.createObject();
  SoftwareSerial debugSerial(10, 11);     // RX, TX
  debugSerial.begin(38400);   //Debug output. Listen on this ports for debugging info
  //debugSerial.println("Send Pairs init: "+ pairs);

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
      // debugSerial.print("DEBUG: else,");
      // debugSerial.println(temp);
    }
  }
  int no = tempPtr-temp;
  strncpy(jsonPtr, temp, no);
  jsonPtr += no;
  //no comma at the end of last JSON value
  *jsonPtr++ = '}';
  *jsonPtr++ = '\0';

  return;
}

/*
 This function will take an ASCII String message and fragment it into 11 byte packets

 2 bytes of header, and 9 bytes of payloads

 header is of the format [fragment number][total number of fragments]
 */
 void LoRaAT::_createFragmentBuffer(char* message) {

  SoftwareSerial debugSerial(10, 11);     // RX, TX
  debugSerial.begin(38400);   //Debug output. Listen on this ports for debugging info
  //debugSerial.println("_create Frag Buff");

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
    //TODO: What to do? For now, just don't send all of it...
    numFragments = _MAX_FRAGMENTS;
  }

  //Create each fragment. TODO: Can we optimise out using char array?
  // char messageAsC[numFragments * _PACKET_SIZE];
  // message.toCharArray(messageAsC,numFragments*_PACKET_SIZE);

  //debugSerial.println("_create Frag Buff, for");
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

    //debugSerial.println("_create Frag Buff, for, for");
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
  }
}

int LoRaAT::_processBuffer() 
{
  SoftwareSerial debugSerial(10, 11);     // RX, TX
  debugSerial.begin(38400);   //Debug output. Listen on this ports for debugging info

  debugSerial.println("Manual print");
  for (int i=0; i < _txPutter; i++)
  {
    for (int j=0; j < _PACKET_SIZE; j++)
    {
      debugSerial.print(_txBuffer[i][j], HEX);
    }
    debugSerial.println();
  }

  int response;
  for(_txGetter; _txGetter < _txPutter; _txGetter++ )
  {
    debugSerial.print("DEBUG: Buffer: ");
    debugSerial.print(_txGetter);
    debugSerial.print(" : ");
    debugSerial.println(_txBuffer[_txGetter]);

    response = send(_txBuffer[_txGetter]);
    debugSerial.print("DEBUG: Sent with response: ");
    debugSerial.println(response, DEC);
  }
  _txPutter = 0;
  _txGetter = 0;
  return(response);
}

/*
  Simple find substring within string function.
  int LoRaAT::_findText(String needle, String haystack) {
    int foundpos = -1;
    for (int i = 0; i <= haystack.length() - needle.length(); i++) {
      if (haystack.substring(i,needle.length()+i) == needle) {
        foundpos = i;
      }
    }
    return foundpos;
  }*/

