/*
 This is a test program writen for the Seeeduino Stalker v2.3 and 
 uses the Multitech mDOT LoRa module running the Australian compatable AT
 enabled firmware.
 
 This program,
  * Joins the LoRa Network.
  * Sends predicitable data (a loop count), and temperature  (The DS3231
 does a temperature conversion once every 64 seconds. This is also the
 default for the DS3232.)
*/

/*--------------------------------------------------------------------------------------
  Includes
  --------------------------------------------------------------------------------------*/
#include <LoRaAT.h>               //Include LoRa AT libraray
#include <SoftwareSerial.h>       //Software serial for debug

/*--------------------------------------------------------------------------------------
  Definitions
  --------------------------------------------------------------------------------------*/

SoftwareSerial debugSerial(10, 11);     // RX, TX
LoRaAT mdot(0, &debugSerial);           //Instantiate a LoRaAT object

/*--- setup() --------------------------------------------------------------------------
  Called by the Arduino framework once, before the main loop begins.

  In the setup() routine:
   - Opens serial communication with MDOT
  --------------------------------------------------------------------------------------*/
void setup() {
  int responseCode;
  
  debugSerial.begin(38400);   //Debug output. Listen on this ports for debugging info
  mdot.begin(38400);          //Begin (possibly amongst other things) opens serial comms with MDOT
  debugSerial.println(F("\r\n\r\n++ START ++\r\n\r\n"));


  do {
    responseCode = mdot.join();
    delay(10000);
  } while (responseCode != 0);
  debugSerial.print(F("SETUP : Join result: "));
  debugSerial.println(String(responseCode));
}

/*--- loop() ---------------------------------------------------------------------------
  Main loop called by the Arduino framework
  --------------------------------------------------------------------------------------*/
int count = 1;
void loop() {
  int responseCode;         //Response code from the mdot
  //String testMessage = "";  //Test message sent via debugSerial
  String temp PROGMEM = "Temp:";
  String rh PROGMEM = ",RH:";
  String testMsg = temp + count + rh + count * 2;
  char testChars[100];

  debugSerial.println("MAIN  : testMsg: " + testMsg);

  testMsg.toCharArray(testChars,50);
  responseCode = mdot.sendPairs(testChars);

  debugSerial.println("MAIN  : send result: " + String(responseCode));

  delay(30000);
  count++;
}
