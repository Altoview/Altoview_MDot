/*
  This is an example program writen for the Arduino UNO R3 and
  uses the Multitech mDOT LoRa module running the Australian 
  compatable AT enabled firmware.

  This program,
    Joins the LoRa Network
    Sends loop count
*/

/*--------------------------------------------------------------------------------------
  Includes
  --------------------------------------------------------------------------------------*/
#include <AltoviewMDot.h>           //Include LoRa AT library
#include <AltSoftSerial.h>          //A better alternative for SoftwareSerial library

/*--------------------------------------------------------------------------------------
  Definitions
  --------------------------------------------------------------------------------------*/
/* Library uses software serial to communicate with the mDot module */
AltSoftSerial mdotSerial;				//AltSoftSerial locks ports 8, 9 for RX, TX
/* Library uses hardware serial to print the debuggin information */
HardwareSerial& debugSerial = Serial;

/* creating an object of a type LoRaAT called mDot */
AltoviewMDot mdot(&mdotSerial, &debugSerial);        //Instantiate a LoRaAT object

/*--- setup() --------------------------------------------------------------------------
  Called by the Arduino framework once, before the main loop begins.

  In the setup() routine:
   - Opens serial communication with MDOT
  --------------------------------------------------------------------------------------*/
void setup() {
  int responseCode;                     // Response of mDot commands
  debugSerial.begin(38400);             // Begins a serial communication of a hardware serial
  mdotSerial.begin(38400);
  
  debugSerial.println("Joining to Altoview...");
  mdot.begin();                    // Begins a serial communication of a software serial

  do {
    responseCode = mdot.join();         // Attempt to join to Altoview
    delay(10000);                       // Wait for the join process to finish.
  } while (responseCode != 0);
}

/*--- loop() ---------------------------------------------------------------------------
  Main loop called by the Arduino framework
  --------------------------------------------------------------------------------------*/
int loopNum = 0;
void loop() {
  int responseCode;                     //Response code from the mdot
  responseCode = mdot.sendPairs("L:" + String(loopNum));
  delay(120000);
  loopNum++;
}
