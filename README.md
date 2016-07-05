# mdot_lora_arduino
The library to send user data to Multitech mDot using AT commands on serial. 

# Pre-requisites
- Make sure the mDot serial port is configured to match the baud rate specified in `begin(<baud-rate>)` methods.
  1. `AT+IPR 38400` to change the BR to 38400 in this example.
  2. `AT&W` to save the changes.
- This library only supports boards with an AVR or SAM processors.

#How to use
- Clone the project to a preferred folder.
- Copy the LoRaAT folder to your `C:\Users\{user}\Documents\Arduino\libraries` directory which Arduino application will access it's libraries.
- Include the library in your project. `#include <LoRaAT.h>`
- Instantiate the mDot object. `LoRaAT mdot;`
- In the `setup()` loop, initialise the instance. `mdot.begin();`
- Now you can use `join()`, `sendPairs()`, etc.

#join( )
This method sends a join request to the LoRa server.

#sendPairs( )
This method accepts a comma separated key-value pair in the form of a `String` or array of chars.
