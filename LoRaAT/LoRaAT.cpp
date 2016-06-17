/*
  LoRaAT.cpp - Arduino library for controlling LoRa(WAN?) modules using AT commands.
  
  This library has specifically created to control the MDOT [TODO: Exactly what it is]
  which is running the  Australian compatable, AT command enabled [TODO: Can I be more
  specific?] firmware.
*/

/************************************************************************************
 *                                   INCLUDES                                       *
 ***********************************************************************************/
#include "Arduino.h"
#include "LoRaAT.h"
#include <ArduinoJson.h>

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
  CONSTRUCTOR: Creates class object using a default serial port 0
*/
LoRaAT::LoRaAT() {
  _u8SerialPort = 0;
}

/*
  CONSTRUCTOR: Creates class object using a specified serial port
*/
LoRaAT::LoRaAT(uint8_t u8SerialPort) {
  //TODO: Input checking, what range of values to accept, how to handle invalid input
  _u8SerialPort = u8SerialPort;
}

/*
  Initialize class object.
  
  Sets up the serial port using default 19200 baud rate.
  Call once class has been instantiated, typically within setup().
*/
void LoRaAT::begin(void) {
  //TODO: Input checking??
  begin(19200);
}

/*
  Initialize class object.
  
  Sets up the serial port using specified baud rate.
  Call once class has been instantiated, typically within setup().

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
  const int LOOP_DELAY = 250;			//Millisecond delay between serial attempts
  unsigned long timeoutCounter = 0;		//We don't wait forever for a response
  String _recievedString;				//String returned by device
  
  //flush receive buffer before transmitting request
  while (MBSerial->read() != -1);
  
  //Send join request
  MBSerial->println("at+join");
  
  //Loop reading from serial buffer until we get either a recognisable error, or
  //success
  timeoutCounter = 0;
  while(timeoutCounter < timeout) {
    if (MBSerial->available() != 0) {
      _recievedString = MBSerial->readString();
	  if (_findText("Failed to join network",_recievedString) != -1) {
		return(1);
	  } else if (_findText("ERROR",_recievedString) != -1) {
        return(-2);
	  } else if (_findText("Successfully joined network",_recievedString) != -1) {
        return(0);
	  } else {
		//Unknown response??
	  }
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
int LoRaAT::send(String message) {
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
int LoRaAT::send(String message, unsigned int timeout) {
  const int LOOP_DELAY = 250;			//Millisecond delay between serial attempts
  unsigned long timeoutCounter = 0;		//We don't wait forever for a response
  String _recievedString;				//String returned by device
  
  //flush receive buffer before transmitting request
  while (MBSerial->read() != -1);
  
  //Send message
  MBSerial->println("at+send " + message);
  
  //Loop reading from serial buffer until we get either a recognisable error, or
  //success
  pinMode(3,OUTPUT);
  timeoutCounter = 0;
  while(timeoutCounter < timeout) {
    if (MBSerial->available() != 0) {
      _recievedString = MBSerial->readString();
	  debug = _recievedString;
	  if (_findText("ACK not received",_recievedString) != -1) {
		return(1);
      } else if (_findText("ERROR",_recievedString) != -1) {
        return(-2);
	  } else if (_findText("Data exceeds datarate max payload",_recievedString) != -1) {
        return(-3);
	  } else if (_findText("OK",_recievedString) != -1) {
        return(0);
	  } else {
		//Unknown response??
	  }
	}
    timeoutCounter += LOOP_DELAY;
	digitalWrite(3, HIGH);
    delay(LOOP_DELAY);
	digitalWrite(3, LOW);
    delay(LOOP_DELAY);
  }
  
  return(-1);
}

/*
  I believe there maybe a ping function... I'm not sure yet what the AT command
  returns, or what we should return to the user?
*/
uint8_t LoRaAT::ping() {

}

/*
 Recieves a string in the format key:value,key:value,...

 1. The csv is translated to a json,
 2. The json is added to the _txBuffer
 3. The _txBuffer is processed

 //TODO: If not recieved in that format return an error
 */
int LoRaAT::sendPairs(String pairs) {
  //Return constants
  const byte UNKNOWN_FORMAT = 4;
  int response = 0;

  String json;      
  
  //TODO: Check the string is actually pairs
  
  if (false) { //TODO: if format not recoginised
    return(UNKNOWN_FORMAT);
  }

  json = _pairsToJSON(pairs);
  _createFragmentBuffer(json);
  response = _processBuffer();

  return(response);
}

/*
 This function will take any correctly formatted string of key:value pairs
 and return a JSON formatted String.
 */
String LoRaAT::_pairsToJSON(String pairs) {
  String json = "";   //Finished JSON as a string
  String temp = "";   //Temp storage as we look for delimters
  String key = "";    //temp storage for the "key".. "value" is pulled straiged from temp
  
  StaticJsonBuffer<_jsonMemeoryPool> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  //Loop through each of the characters, when getting to a delimiter, act accordingly
  for (int i = 0; i < pairs.length(); i++) {
    char c = pairs.charAt(i);
    if (c == ':') {
      key = temp;
      temp = "";
    } else if (c == ',') {
      root[key] = temp.toFloat();               //Defaults to 2 point precision
      temp = "";
    } else {
      temp += c;
    }
  }
  //Of course add the final key value which is not succeeded by a delimter ","
  root[key] = temp.toFloat();

  //Write the json to a string
  root.printTo(json);

  return(json);
}

/*
 This function will take and ASCII String message and fragment it into 11 byte packets

 2 bytes of header, and 9 bytes of payloads

 header is of the format [fragment number][total number of fragments]
 */
void LoRaAT::_createFragmentBuffer(String message) {

  //Calculate the number of fragments required for this message
  int numFragments = message.length()/_PAYLOAD_SIZE;
  
  //Make sure we round up
  if (_PAYLOAD_SIZE*numFragments < message.length()) {
    numFragments += 1;
  }

  //Check we haven't exceeded the maximum number of fragments
  if (numFragments > _MAX_FRAGMENTS) {
    //TODO: What to do? For now, just don't send all of it...
    numFragments = _MAX_FRAGMENTS;
  }

  //Create each fragment. TODO: Can we optimise out using char array?
  char messageAsC[numFragments*_PACKET_SIZE];
  message.toCharArray(messageAsC,numFragments*_PACKET_SIZE);

  //Loop through each fragment
  for (_txPutter = 0; _txPutter < numFragments; _txPutter++) {
    union headerPacket {
      uint8_t asByte[2];
      char asChar[2];
    } header;

    //Create the header bytes
    header.asByte[0] = _txPutter + 1;
    header.asByte[1] = numFragments;

    //Add the header to the byte array (delt with as a string)
    _txBuffer[_txPutter] = header.asChar[0];
    _txBuffer[_txPutter] += header.asChar[1];

    //Loop through each location of the message and append to the fragment (as the payload)
    for (int j = 0; j < _PAYLOAD_SIZE; j++) {
      _txBuffer[_txPutter] += messageAsC[_PAYLOAD_SIZE*_txPutter + j];        
      if (messageAsC[_PAYLOAD_SIZE*_txPutter+j] == '\0') {
        break;
      }
    }
  }
}

int LoRaAT::_processBuffer() {
  int response = 0;
  response = send(_txBuffer[0]);
  response = send(_txBuffer[1]);
  _txPutter = 0;
  _txGetter = 0;
  return(response);
}

/*
  Simple find substring within string function.
*/
int LoRaAT::_findText(String needle, String haystack) {
  int foundpos = -1;
  for (int i = 0; i <= haystack.length() - needle.length(); i++) {
    if (haystack.substring(i,needle.length()+i) == needle) {
      foundpos = i;
    }
  }
  return foundpos;
}