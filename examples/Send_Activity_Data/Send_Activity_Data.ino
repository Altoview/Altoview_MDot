/*
   21 Mar 17 - Dane Lennon
   v0.2

   This is a script writen for the Arduino Uno for the CSA Student Day and
   uses the Multitech mDOT LoRa module running the Australian compatable AT
   enabled firmware.

   Includes:
    - Altoview mDOT: https://github.com/Altoview/Altoview_MDot
    - AltSoftSerial: https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html to send data to the mDot via a serial port other than the Hardware Serial


   This program,
    Joins the LoRa Network
    Waits for motion to be sensed
    Sends the following fields:
      - count: # of ticks in the last 30 seconds or since last successful send 
      - accum: total pulse length time in the last 30 seconds or since last successful send
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
AltSoftSerial mdotSerial;        // AltSoftSerial only uses ports 8, 9 for RX, TX
/* library uses hardware serial to print the debuggin information */
HardwareSerial& debugSerial = Serial;

/* creating an object of a type LoRaAT called mDot */
AltoviewMDot mdot(&mdotSerial, &debugSerial);

int interruptPin = 2;                                 //attach the OUT pin of the PIR to pin 2 on the Uno
int count = 0;
int responseCode;                              //Response code from the mdot
char msg[15];                                  //cmd = {'a', 'l', 'e', 'r', 't', ':', '#', ',',
//       'c', 'o', 'u', 'n', 't', ':', '#', '#', '#'}
unsigned long rise, fall, pulseLength;
unsigned long pulseAccumulator = 0;
boolean rising = false;

/*--- setup() --------------------------------------------------------------------------
  Called by the Arduino framework once, before the main loop begins.

  In the setup() routine:
   - Opens serial communication with MDOT
  --------------------------------------------------------------------------------------*/
void setup() {
  pinMode(interruptPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(interruptPin), updateCount, CHANGE);

  int responseCode;
  /* begins a serial communication of a hardware serial */
  debugSerial.begin(38400);
  /* begins a serial communication of a software serial */
  mdotSerial.begin(38400);

  debugSerial.println("\n\nJoining Altoview...\n\n");
  mdot.begin();

  do {
    /* attempt to join to Altoview */
    responseCode = mdot.join();
    /* waiting for the join process to finish. */
    delay(1000);
    if (responseCode == -1) {
      /* To go around the Join Backoff if received from the LoRa Server */ 
      delay(120000);              
    }
  } while (responseCode != 0);
  
  /* Send a simple loop count to confirm that Altoview is set up correctly and receiving this sample data. */
  sprintf(msg, "count:%d", count);
  responseCode = mdot.sendPairs(msg);
  if (responseCode == 0) {
    debugSerial.println("Success");
  } else {
    debugSerial.println("Fail");
  }
}

/*--- loop() ---------------------------------------------------------------------------
  Main loop called by the Arduino framework
  --------------------------------------------------------------------------------------*/
void loop() {
  int accum = pulseAccumulator/1000; 
  responseCode = -1;
  sprintf(msg, "count:%d,accum:%d", count, accum);
  responseCode = mdot.sendPairs(msg);
  if (responseCode == 0)      // Successful send.
  {
    count = 0;                // Reset the count to 0 to record the number of triggers since last packet sent.
    pulseAccumulator = 0;
  }
  delay(30000);
}

void updateCount() {
  if (digitalRead(interruptPin)) {
    rising = true;
    rise = millis();
  } else {
    if (rising) {                         // Only execute this if we have had a rising edge previously. (set to false initially) 
      fall = millis();
      pulseLength = fall - rise;          // The time between the rising and falling edge of the trigger.
      pulseAccumulator += pulseLength;    // The total time active.
      ++count;
      debugSerial.println(count);
      debugSerial.println(pulseLength);
    }
    rising = false;
  }
}

