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
  pinMode(13, OUTPUT);
  
  debugSerial.begin(38400);   //Debug output. Listen on this ports for debugging info
  mdot.begin(38400);          //Begin (possibly amongst other things) opens serial comms with MDOT
  debugSerial.println("\r\n\r\n++ START ++\r\n\r\n");


  do {
    digitalWrite(13, HIGH);
    responseCode = mdot.join();
    delay(5000);
  } while (responseCode != 0);
  debugSerial.println("DEBUG MAIN: Join result: " + String(responseCode));
}

/*--- loop() ---------------------------------------------------------------------------
  Main loop called by the Arduino framework
  --------------------------------------------------------------------------------------*/
unsigned int count = 1;
void loop() {
  int responseCode;         //Response code from the mdot
  String testMessage = "";  //Test message sent via debugSerial
  char testMsg[40] = "test:";
  sprintf(testMsg, "test:%d,alex:%d", count, count*2);
  debugSerial.print("DEBUG MAIN: testMsg: ");
  debugSerial.println(testMsg);

  responseCode = mdot.sendPairs(testMsg);
  debugSerial.println("send result: " + String(responseCode));
/*
  if (debugSerial.available()) {
    testMessage = debugSerial.readString();
    responseCode = mdot.sendPairs(testMessage);
    debugSerial.println("send result: " + String(responseCode));
  }
  */

  delay(10000);
  count++;
}
