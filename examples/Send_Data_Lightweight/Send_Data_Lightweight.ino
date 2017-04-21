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
/* using software serial to control the mDot module */
AltSoftSerial mdot_serial;//(10, 11);              //RX, TX
/* using hardware serial to print the debuggin information */
HardwareSerial* hardwareSerial= &Serial;
/* creating an object of a type LoRaAT called mDot */
AltoviewMDot mdot(&mdot_serial, hardwareSerial);        //Instantiate a LoRaAT object
/* dereferencing the hardwareSerial for the end user to be able to use "." instead of "->" */
HardwareSerial debugSerial = *hardwareSerial;

/*--- setup() --------------------------------------------------------------------------
  Called by the Arduino framework once, before the main loop begins.

  In the setup() routine:
   - Opens serial communication with MDOT
  --------------------------------------------------------------------------------------*/
void setup() {
  int responseCode;                              //Response of mDot commands
  debugSerial.begin(38400);
  mdot.begin(38400);                                  //Opens serial comms with MDOT

  do {
    responseCode = mdot.join();
    delay(10000);
  } while (responseCode != 0);
}

/*--- loop() ---------------------------------------------------------------------------
  Main loop called by the Arduino framework
  --------------------------------------------------------------------------------------*/
int loopNum = 0;
void loop() {
  int responseCode;                              //Response code from the mdot

  responseCode = mdot.sendPairs("L:" + String(loopNum));

  delay(120000);
  loopNum++;
}
