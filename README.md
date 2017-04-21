# About Arduino_MDot
Contains a library (with example scripts) to send user data to [Multitech mDot](http://www.multitech.com/brands/multiconnect-mdot) using [AT commands](https://www.sparkfun.com/datasheets/Cellular%20Modules/AT_Commands_Reference_Guide_r0.pdf) on serial. All AT commands begin with an `AT`. Please see the link for information on AT commands. 

# Note well: 
- The mDot serial port Baud Rate (BR) should match the BR specified in the `begin(<baud-rate>)` method.
  1. `AT+IPR 38400` to change the BR to 38400 in this example.
  2. `AT&W` to save the changes.
- This library **only** supports boards with an **AVR or SAM processor**.

## How to use
- Clone the project to a preferred folder.
- Copy the ArduinoMDot folder to your Arduino directory (eg: `C:\Users\{user}\Documents\Arduino\libraries`). This allows the Arduino application to access it's libraries.
- Create a new Arduino script (ensure to set the BR of the serial to the chosen BR).
- Include the library in your new Arduino: `#include <ArduinoMDot.h>`
- Before the `setup()` instruction, instantiate the mDot object: `ArduinoMDot mdot;`
- In the `setup()` function, initialise an instance of the mdot object: `mdot.begin();` (see [object oriented programming](https://en.wikipedia.org/wiki/Object-oriented_programming) wiki page for more information
- Now you can use the instance methods  `mdot.join()` and `mdot.sendPairs()` etc 
  1. `mdot.join()` joins the LoRa network and returns a response code. If the response code is equal to 0 then the join has not been successful and you will need to run the `mdot.join()` command again. 
  2. Once the `mdot.join()` method is successful, the `mdot.networkSessionKey` method will return the relevant session key
- Please see the example Arduino scripts (supplied in the `ArduinoMDot/examples` folder) for further information on how to connect to the LoRa network and send data

### join( )
This method sends a join request to the LoRa server.

### sendPairs( )
This method accepts a comma separated key-value pair in the form of a `String` or `array of chars`
- example:
`Temp1:26.00,Temp2:22.70,RH%:59,Moral%:100`

### Wiring and debugging: 
- Once you have uploaded the code to your dev board, you must remove the TX and RX wires (otherwise this will disrupt the mdot signals being sent to the LoRa netork and you node will not connect) 
- Ensure that all your baudrates match. Everything must **send on** and **receive on** on 34100 (or the baudrate in the `begin()` statement)
- The mDot must be programmed with the Australian Firmware 
- The baudrate is effected by the speed of the On Board Crystal. If the microcontroller is programmed at 8 Mhz then the dev board performing the serial listening task for the debug serial must also have 8 Mhz 

#### The debug serial
If you have chosen to use a debug serial, it should be noted that the serial **is not** instantiated in the usual way. Instead, the LoRat object must be made aware of the extra serial using the syntax: 

  ```
  SoftwareSerial debugSerial(10, 11); // RX, TX 
  ArduinoMDot mdot(0, &debugSerial);     //Instantiate a ArduinoMDot object
  ```
  
The mdot object will then post data to the serial on its own accord. An explanation of some likely debug responses is listed below: 
```
LaT:sp: {"L":66}            --> sp stands for *send pairs* 
LaT:sc: AT+TXDR?            --> sc stands for *send command* and TXDR stands for transmit data rate (the ? is asking for a response) 
LaT:sc: AT+SEND 11{"L":66}  --> the pairs of data is sent 
```
For more information on AT commands, please see the Multech [AT Command Getting Started Guide](http://www.multitech.net/developer/software/mdot-software/at-command-firmware-user-guide/) and the [DOT Series AT Command Reference Guide](http://www.multitech.com/documents/publications/manuals/s000643.pdf).

### Register Node in AltoView
- Once you have identified the LoRa node has successful connected to the LoRa network and is sending data, navigate to [AltoView](http://www.altoview.com/) (create a login if you don't already have one) 
- On the left hand side menu, select **My Nodes** and click **Register a Node** 
- Enter the DevEUI ([Development Extended Unique Identifier](https://en.wikipedia.org/wiki/MAC_address)) which can be found on the mDot node in small print (look for a 16 value long ID with numbers and letters in it, eg: 00 80 00 00 00 00 AA 66) 

#### NB when registering node in AltoView 
- although the DevEUI is written in *uppercase* on most mDot modules, altoview requirers the DevEUI in ***LOWERCASE*** 
- once the node details have been entered and you have hit the `register` button, the node will have either an `online` or `offline` status. Please note that this status **does not** mean that the node is powered on and connected to the LoRa network. Instead: 
  - `online` --> the node was connected and sending data in the past. In other words, there is a history of the node having sent data
  - `offline` --> the node has never sent data. In other words, AltoView has never seen this nodes devEUI before 


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
