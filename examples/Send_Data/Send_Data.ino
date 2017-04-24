/*
  This is an example program writen for the Arduino UNO R3 and
  uses the Multitech mDOT LoRa module running the Australian 
  compatable AT enabled firmware.

  This program,
    Joins the LoRa Network
    Sends a loop count to Altoview
*/

/*--------------------------------------------------------------------------------------
  Includes
  --------------------------------------------------------------------------------------*/
#include <AltoviewMDot.h>
#include <AltSoftSerial.h>

/*--------------------------------------------------------------------------------------
  Definitions
  --------------------------------------------------------------------------------------*/
/* library uses software serial to communicate with the mDot module */
AltSoftSerial mdotSerial;				// AltSoftSerial only uses ports 8, 9 for RX, TX 
/* library uses hardware serial to print the debuggin information */
HardwareSerial& debugSerial = Serial;

/* creating an object of a type LoRaAT called mDot */
AltoviewMDot mdot(&mdotSerial, &debugSerial);

/*--- setup() --------------------------------------------------------------------------
  Called by the Arduino framework once, before the main loop begins.

  In the setup() routine:
   - Opens serial communication with MDOT
  --------------------------------------------------------------------------------------*/
void setup() {
  int responseCode;
  /* begins a serial communication of a hardware serial */
  debugSerial.begin(38400);             
  /* begins a serial communication of a software serial */
  mdotSerial.begin(38400);
  
  debugSerial.println("Joining to Altoview...");
  mdot.begin();							

  do {
	/* attempt to join to Altoview */
    responseCode = mdot.join();
	/* waiting for the join process to finish. */
    delay(10000);
  } while (responseCode != 0);
}

/*--- loop() ---------------------------------------------------------------------------
  Main loop called by the Arduino framework
  --------------------------------------------------------------------------------------*/
int loopNum = 0;
void loop() {
  int responseCode;
  /* send the loop count in the JSON format to Altoview */
  responseCode = mdot.sendPairs("L:" + String(loopNum));
  /* wait for 2 mins */
  delay(120000);						
  loopNum++;
}
