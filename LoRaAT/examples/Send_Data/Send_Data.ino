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
#include <Wire.h>
#include "DS3231.h"

/*--------------------------------------------------------------------------------------
  Definitions
  --------------------------------------------------------------------------------------*/

SoftwareSerial debugSerial(10, 11);     // RX, TX
LoRaAT mdot(0, &debugSerial);           //Instantiate a LoRaAT object
DS3231 RTC;                             //Create the DS3231 object

/*--- setup() --------------------------------------------------------------------------
  Called by the Arduino framework once, before the main loop begins.

  In the setup() routine:
   - Opens serial communication with MDOT
  --------------------------------------------------------------------------------------*/
void setup() {
  int responseCode;
  
  debugSerial.begin(38400);   //Debug output. Listen on this ports for debugging info
  mdot.begin(38400);          //Begin (possibly amongst other things) opens serial comms with MDOT
  Wire.begin();
  RTC.begin();
  
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
int loopNum = 0;
void loop() {
  int responseCode;         //Response code from the mdot
  String testMessage = "";  //Test message sent via debugSerial
  
  //Build the message to send:
  String rtcTemp = F("Temp1:");
  testMessage = rtcTemp;
  RTC.convertTemperature(); //convert current temperature into registers
  testMessage += RTC.getTemperature();

  String atTemp = F(",Temp2:");
  testMessage += atTemp;
  testMessage += GetTemp();
  
  String count = F(",Loop:");
  testMessage += count;
  testMessage += loopNum;

  DateTime now = RTC.now(); //get the current date-time
  String Year = F(",Year:");
  testMessage += Year;
  testMessage += now.year();
  
  String Month = F(",Month:");
  testMessage += Month;
  testMessage += now.month();
  
  String Day = F(",Day:");
  testMessage += Day;
  testMessage += now.date();

  String Hour = F(",Hour:");
  testMessage += Hour;
  testMessage += now.hour();
  
  String Min = F(",Min:");
  testMessage += Min;
  testMessage += now.minute();
  
  String Sec = F(",Sec:");
  testMessage += Sec;
  testMessage += now.second();
  
  debugSerial.println("MAIN  : testMsg: " + testMessage);

  responseCode = mdot.sendPairs(testMessage);

  debugSerial.println("MAIN  : send result: " + String(responseCode));

  delay(30000);
  loopNum++;
}

double GetTemp(void) {
  unsigned int wADC;
  double t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celsius.
  return (t);
}
