/*
  File: LoRaAT.h

  Version: v0.0.1.1

  Brief: Arduino library for controlling Multitech mDot LoRa modules using
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
    char networkKey[48] = {'\0'};                 //00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01
    char networkId[24] = {'\0'};                  //00:00:aa:00:00:00:00:01
    char frequencySubBand = '\0';                 //0-8
    char publicNetwork = '\0';                    //0,1
    char dataRate = '\0';                         //0-3
    char adaptiveDataRate = '\0';                 //0,1

    LoRaAT();                                     //Use default serial port
    LoRaAT(uint8_t);                              //Use specified serial port.
    LoRaAT(uint8_t,Stream*);                      //Use specified serial port and a debugging stream.

    void begin();                                 //Use default baud
    void begin(uint32_t);                         //Use specified baud rate.

    int8_t join();                                //Join a LoRa network.
    int8_t join(uint16_t);                        //Join with a specific timeout
    void leave();                                 //Leave a LoRa network, Not yet implemented

    int8_t send(char*);                           //Generic send command, using AT+SEND
    int8_t send(char*, uint16_t);                 //Use specific timeout.
    int8_t send(char*, uint8_t, uint16_t);        //Use specific message length.
    int8_t sendPairs(char*);                      //Takes key,value pairs, forms a message, and sends to the LoRa server.
    int8_t sendPairs(String);                     //Takes key,value pairs, forms a message, and sends to the LoRa server.
    int8_t ping();                                //Not yet implemented

    int8_t setDefaults();
    int8_t setFrequencySubBand(char);
    int8_t getFrequencySubBand();
    int8_t setPublicNetwork(char);
    int8_t getPublicNetwork();
    int8_t setNetworkID(char*);                   //Also referred to as the AppEUI
    int8_t getNetworkID();                        //Also referred to as the AppEUI
    int8_t setNetworkKey(char*);                  //Also referred to as the AppKey
    int8_t getNetworkKey();                       //Also referred to as the AppKey
    int8_t setDataRate(char);
    int8_t getDataRate();
    int8_t setAdaptiveDataRate(char);
    int8_t getAdaptiveDataRate();
    int8_t commitSettings();

  private:
    static const uint8_t _DR0_PAYLOAD_USAGE = 11;     //Use 11 of 11 available bytes in payload
    static const uint8_t _DR1_PAYLOAD_USAGE = 53;     //Use 53 of 53 available bytes in payload
    static const uint8_t _DR2_PAYLOAD_USAGE = 126;    //Use 126 of 126 available bytes in payload
    static const uint8_t _DR3_PAYLOAD_USAGE = 126;    //Use 126 of 242 available bytes in payload
    static const uint8_t _DR4_PAYLOAD_USAGE = 126;    //Use 126 of 242 available bytes in payload

    static const uint8_t _MAX_FRAGMENTS = 16;
    static const uint8_t _PACKET_SIZE = 11;
    static const uint8_t _HEADER_SIZE = 2;
    static const uint8_t _PAYLOAD_SIZE = _PACKET_SIZE - _HEADER_SIZE;
    static const uint8_t _HEADER_OFFSET = 48;

    static const uint8_t _MAX_PAIRS_SIZE = 100;

    char _txBuffer[_MAX_FRAGMENTS][_PACKET_SIZE];
    uint8_t _txPutter = 0;                        //Number of fragments in buffer. TODO: Replace/rename

    static const uint8_t _MAX_MDOT_RESPONSE = 200;//Max number of bytes the mdot might return
    char _response[_MAX_MDOT_RESPONSE];           //mDot response buffer
    uint8_t _length;                              //Lenght of a response

    static const uint8_t _MAX_MDOT_COMMAND = 150; //TODO: Check against the manual for mDot
    char _command[_MAX_MDOT_COMMAND];

    int8_t _sendCommand(char*, char*, char*, char*, char*, uint16_t);
    int8_t _sendCommand(char*, char*, char*, char*, char*, uint16_t, char**);   //Generic serial out get response wrapper

    void _pairsToJSON(char*, uint8_t, char*);
    void _createFragmentBuffer(char*);
    int8_t _processBuffer();

    uint8_t _u8SerialPort;                        //AT Command serial port selection by user
    Stream* _debugStream;                         //Debugging serial port initialized in constructor
};

#endif
