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
#include <SoftwareSerial.h>                      //Software serial for debug
#include <EEPROM.h>
#include <Wire.h>
#include "DS3231.h"                              //http://www.seeedstudio.com/wiki/Seeeduino_Stalker_v2.3#Real_Time_Clock_.28RTC.29_Related
                                                 //https://github.com/bpg/DS3231

/*--------------------------------------------------------------------------------------
  Definitions
  --------------------------------------------------------------------------------------*/
SoftwareSerial debugSerial(10, 11);              //RX, TX
LoRaAT mdot(0, &debugSerial);                    //Instantiate a LoRaAT object
DS3231 RTC;                                      //Create the DS3231 object

const uint32_t MAX_SESSION_AGE = 345600;         //Max session age allowed in seconds. e.g. 4 days => 4*24*60*60 = 345600
DateTime loraSessionStart;                       //Time the LoRa session began

/*--- setup() --------------------------------------------------------------------------
  Called by the Arduino framework once, before the main loop begins.

  In the setup() routine:
   - Opens serial communication with MDOT
  --------------------------------------------------------------------------------------*/
void setup() {
  debugSerial.println(F("\r\n\r\n++ START ++\r\n\r\n"));

  int responseCode;                              //Response of mDot commands
  
  debugSerial.begin(38400);                      //Debug output. Listen on this software serial port for debugging info
  mdot.begin(38400);                             //Opens serial comms with MDOT
  Wire.begin();
  RTC.begin();

  mdot.restoreLoraSession();                     //Restore any saved session on mDot

  //Debug feedback for the developer to double check the session information obtained by the Arduino
  debugSerial.print(F("SETUP : Network Session Key: "));
  debugSerial.println(mdot.networkSessionKey);
  debugSerial.print(F("SETUP :   Data Sesstion Key: "));
  debugSerial.println(mdot.dataSessionKey);

  EEPROM.get(1,loraSessionStart);                //Check the Arduino EEPROM for a saved timestamp
  
  //Debug feedback for the developer to double check the time information obtained from the EEPROM
  debugSerial.print(F("SETUP : Timestamp mem: "));
  debugSerial.println(loraSessionStart.get());
  debugSerial.print(F("SETUP : Timestamp now: "));
  debugSerial.println(RTC.now().get());

  //If we have a blank session key, or an old session key, rejoin the LoRa network
  if ((mdot.networkSessionKey == "00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00") || ((RTC.now().get() - loraSessionStart.get() > MAX_SESSION_AGE))) {
    do {
      responseCode = mdot.join();

      //Debug feedback for the developer to double check the result of the join() instruction
      debugSerial.print(F("SETUP : Join result: "));
      debugSerial.println(String(responseCode));
      
      delay(10000);
    } while (responseCode != 0);

    //Debug feedback for the developer to double check the *new* session information obtained by the Arduino
    debugSerial.print(F("SETUP : Network Session Key: "));
    debugSerial.println(mdot.networkSessionKey);
    debugSerial.print(F("SETUP :   Data Sesstion Key: "));
    debugSerial.println(mdot.dataSessionKey);
  
    loraSessionStart = RTC.now();
    EEPROM.put(1,loraSessionStart);

    //Debug feedback for the developer to double check the session information obtained by the Arduino
    debugSerial.print(F("SETUP : Timestamp mem: "));
    debugSerial.println(loraSessionStart.get());
  }
}

/*--- loop() ---------------------------------------------------------------------------
  Main loop called by the Arduino framework
  --------------------------------------------------------------------------------------*/
int loopNum = 0;
void loop() {
  int responseCode;                              //Response code from the mdot
  String testMessage = "";                       //Test message sent via debugSerial

  DateTime now = RTC.now();                      //get the current date-time

  //If we have an old session key, rejoin the LoRa network
  if (now.get() - loraSessionStart.get() > MAX_SESSION_AGE) {
    do {
      responseCode = mdot.join();

      //Debug feedback for the developer to double check the result of the join() instruction
      debugSerial.print(F("SETUP : Join result: "));
      debugSerial.println(String(responseCode));
      
      delay(10000);
    } while (responseCode != 0);

    //Debug feedback for the developer to double check the *new* session information obtained by the Arduino
    debugSerial.print(F("SETUP : Network Session Key: "));
    debugSerial.println(mdot.networkSessionKey);
    debugSerial.print(F("SETUP :   Data Sesstion Key: "));
    debugSerial.println(mdot.dataSessionKey);
  
    loraSessionStart = RTC.now();
    EEPROM.put(1,loraSessionStart);

    //Debug feedback for the developer to double check the session information obtained by the Arduino
    debugSerial.print(F("SETUP : Timestamp mem: "));
    debugSerial.println(loraSessionStart.get());
  }
  
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

  //Debug feedback for the developer to double check what the library will try to send over the LoRaWAN
  debugSerial.println("MAIN  : " + testMessage);

  responseCode = mdot.sendPairs(testMessage);

  //Debug feedback for the developer to double check what the result of the send
  debugSerial.println("MAIN  : send result: " + String(responseCode));

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
