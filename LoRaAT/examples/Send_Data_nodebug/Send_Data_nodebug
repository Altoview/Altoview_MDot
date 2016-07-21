/*
 This is an example program writen for the Seeeduino Stalker v2.3 and 
 uses the Multitech mDOT LoRa module running the Australian compatable AT
 enabled firmware.
 
 This program,
  * Checks for saved sessions.
  * Joins the LoRa Network if:
    * There is no saved session, or
    * The saveed session is too old.
  * Sends example data:
    * Temperature from the DS3231,
    * Temperature from the ATmega382p,
    * The loop count, and
    * Time from the DS3231
*/

/*--------------------------------------------------------------------------------------
  Includes
  --------------------------------------------------------------------------------------*/
#include <LoRaAT.h>                              //Include LoRa AT libraray
#include <EEPROM.h>
#include <Wire.h>
#include "DS3231.h"                              //http://www.seeedstudio.com/wiki/Seeeduino_Stalker_v2.3#Real_Time_Clock_.28RTC.29_Related
                                                 //https://github.com/bpg/DS3231

/*--------------------------------------------------------------------------------------
  Definitions
  --------------------------------------------------------------------------------------*/
LoRaAT mdot;                                     //Instantiate a LoRaAT object
DS3231 RTC;                                      //Create the DS3231 object

const uint32_t MAX_SESSION_AGE = 345600;         //Max session age allowed in seconds. e.g. 4 days => 4*24*60*60 = 345600
DateTime loraSessionStart;                       //Time the LoRa session began

/*--- setup() --------------------------------------------------------------------------
  Called by the Arduino framework once, before the main loop begins.

  In the setup() routine:
   - Opens serial communication with MDOT
  --------------------------------------------------------------------------------------*/
void setup() {
  int responseCode;                              //Response of mDot commands
  
  mdot.begin(38400);                             //Opens serial comms with MDOT
  Wire.begin();
  RTC.begin();

  mdot.restoreLoraSession();                     //Restore any saved session on mDot

  EEPROM.get(1,loraSessionStart);                //Check the Arduino EEPROM for a saved timestamp

  //If we have a blank session key, or an old session key, rejoin the LoRa network
  if ((mdot.networkSessionKey == "00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00") || ((RTC.now().get() - loraSessionStart.get() > MAX_SESSION_AGE))) {
    do {
      responseCode = mdot.join();
      delay(10000);
    } while (responseCode != 0);

    loraSessionStart = RTC.now();
    EEPROM.put(1,loraSessionStart);
  }
}

/*--- loop() ---------------------------------------------------------------------------
  Main loop called by the Arduino framework
  --------------------------------------------------------------------------------------*/
int loopNum = 0;
void loop() {
  int responseCode;                              //Response code from the mdot
  String testMessage = "";                       //Test message sent via debugSerial
  
  //Build the message to send:
  String rtcTemp = F("Temp RTC:");
  testMessage = rtcTemp;
  RTC.convertTemperature();                      //convert current temperature into registers
  testMessage += RTC.getTemperature();

  String atTemp = F(",Temp ATmega:");
  testMessage += atTemp;
  testMessage += GetTemp();
  
  String count = F(",Loop Count:");
  testMessage += count;
  testMessage += loopNum;

  DateTime now = RTC.now();                      //get the current date-time
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

  responseCode = mdot.sendPairs(testMessage);

  delay(120000);
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
