/*
  This is an example program writen for the Seeeduino Stalker v2.3 and
  uses the Multitech mDOT LoRa module running the Australian compatable AT
  enabled firmware.

  This program,
    Joins the LoRa Network
    Sends loop count
*/

/*--------------------------------------------------------------------------------------
  Includes
  --------------------------------------------------------------------------------------*/
#include <LoRaAT.h>                              //Include LoRa AT libraray

/*--------------------------------------------------------------------------------------
  Definitions
  --------------------------------------------------------------------------------------*/
LoRaAT mdot;                                     //Instantiate a LoRaAT object

/*--- setup() --------------------------------------------------------------------------
  Called by the Arduino framework once, before the main loop begins.

  In the setup() routine:
   - Opens serial communication with MDOT
  --------------------------------------------------------------------------------------*/
void setup() {
  int responseCode;                              //Response of mDot commands
  mdot.begin();                                  //Opens serial comms with MDOT

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
