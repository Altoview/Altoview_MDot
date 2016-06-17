/*
 This is a test program is writen for the Seeeduino Stalker v2.3 and 
 uses the MDOT LoRa(WAN?) module running the Australian compatable AT
 enabled firmware.
 
 This program joins the LoRa Network.
 
 Sends predicitable data (a loop count), and temperature  (The DS3231
 does a temperature conversion once every 64 seconds. This is also the
 default for the DS3232.)
*/

/*--------------------------------------------------------------------------------------
  Includes
  --------------------------------------------------------------------------------------*/
#include <LoRaAT.h>               //Include LoRa AT libraray
#include <SoftwareSerial.h>       //Sneaky sofware serial to output debug info

/*--------------------------------------------------------------------------------------
  Definitions
  --------------------------------------------------------------------------------------*/

LoRaAT mdot;                            //Instantiate a LoRaAT object
SoftwareSerial debugSerial(10, 11);     // RX, TX

/*--- setup() --------------------------------------------------------------------------
  Called by the Arduino framework once, before the main loop begins.

  In the setup() routine:
   - Opens serial communication with MDOT
  --------------------------------------------------------------------------------------*/
void setup() {
  int responseCode;

  debugSerial.begin(38400);   //Debug output. Listen on this ports for debugging info
  mdot.begin(38400);          //Begin (possibly amongst other things) opens serial comms with MDOT

  do {
    responseCode = mdot.join();
    delay(5000);
  } while (responseCode != 0);
  debugSerial.println("join result: " + String(responseCode));
}

/*--- loop() ---------------------------------------------------------------------------
  Main loop called by the Arduino framework
  --------------------------------------------------------------------------------------*/
unsigned int count = 0;
void loop() {
  int responseCode;         //Response code from the mdot
  String testMessage = "";  //Test message sent via debugSerial
  
  responseCode = mdot.sendPairs("test:" +String(count));
  debugSerial.println("send result: " + String(responseCode));

  if (debugSerial.available()) {
    testMessage = debugSerial.readString();
    responseCode = mdot.sendPairs(testMessage);
    debugSerial.println("tend result: " + String(responseCode));
  }

  delay(10000);
  count++;
}
