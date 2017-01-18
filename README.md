# About mdot_lora_arduino
Contains a library (with example scripts) to send user data to [Multitech mDot](http://www.multitech.com/brands/multiconnect-mdot) using [AT commands](https://www.sparkfun.com/datasheets/Cellular%20Modules/AT_Commands_Reference_Guide_r0.pdf) on serial. All AT commands begin with an `AT`. Please see the link for information on AT commands. 

# Note well: 
- The mDot serial port Baud Rate (BR) should match the BR specified in the `begin(<baud-rate>)` method.
  1. `AT+IPR 38400` to change the BR to 38400 in this example.
  2. `AT&W` to save the changes.
- This library **only** supports boards with an **AVR or SAM processor**.

## How to use
- Clone the project to a preferred folder.
- Copy the LoRaAT folder to your Arduino directory (eg: `C:\Users\{user}\Documents\Arduino\libraries`). This allows the Arduino application to access it's libraries.
- Create a new Arduino script (ensure to set the BR of the serial to the chosen BR).
- Include the library in your new Arduino: `#include <LoRaAT.h>`
- Before the `setup()` instruction, instantiate the mDot object: `LoRaAT mdot;`
- In the `setup()` function, initialise an instance of the mdot object: `mdot.begin();` (see [object oriented programming](https://en.wikipedia.org/wiki/Object-oriented_programming) wiki page for more information
- Now you can use the instance methods  `mdot.join()` and `mdot.sendPairs()` etc 
  1. `mdot.join()` joins the LoRa network and returns a response code. If the response code is equal to 0 then the join has not been successful and you will need to run the `mdot.join()` command again. 
  2. Once the `mdot.join()` method is successful, the `mdot.networkSessionKey` method will return the relevant session key
- Please see the example Arduino scripts (supplied in the `LoRaAT/examples` folder) for further information on how to connect to the LoRa network and send data

### join( )
This method sends a join request to the LoRa server.

### sendPairs( )
This method accepts a comma separated key-value pair in the form of a `String` or `array of chars`.
- example:
`Temp1:26.00,Temp2:22.70,RH%:59,Moral%:100`

- Once you have identified the LoRa node has successful connected to the LoRa network and is sending data, navigate to [AltoView](http://www.altoview.com/) (create a login if you don't already have one) 
- On the left hand side menu, select **My Nodes** and click **Register a Node** 
- Enter the DevEUI ([Development Extended Unique Identifier](https://en.wikipedia.org/wiki/MAC_address)) which can be found on the mDot node in small print (look for a 16 value long ID with numbers and letters in it, eg: 00 80 00 00 00 00 AA 66) 


License
=======

Copyright (C) 2013-2016 - Campbell Scientific Australia ,Pty Ltd

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as 
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
