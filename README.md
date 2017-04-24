# About `Arduino_MDot`
Contains a library (with example scripts) to send user data to [Multitech mDot](http://www.multitech.com/brands/multiconnect-mdot) using [AT commands](https://www.sparkfun.com/datasheets/Cellular%20Modules/AT_Commands_Reference_Guide_r0.pdf) on serial. All AT commands begin with an `AT`. Please see the link for information on AT commands. 

# Update History #
- v0.2.1
	- *[Hotfix]* This fix is applied to separate the commands list from the ID lists. This should not affect the end user.
	- The `README.md` file is updated with the latest changes.
- v0.2
	- We have changed the software serial ([AltSoftSerial](https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html)) to run the MDot radio and using hardware serial for printing the debugging information.
	- The change above, enables the use of serial monitor in the Arduino IDE and also does not require the XBee module to be disconnected while user is attempting to program the Arduino. 
	- We are using AltSoftSerial instead of the traditional SoftwareSerial library because of its advantages such as supporting full-duplex and lower CPU burden.
	-  **Please note** that this version of the library is not backward-compatible with the previous releases.
-  v0.1
	- The library enables you to transmit the information in the correct JSON packets to the Altoview platform.
	- This is the initial release of the library and plenty of hotfixes are under development at this point in time.

## Note well: 
- The mDot serial port Baud Rate (BR) should match the BR specified in the `begin(<baud-rate>)` method.
  1. `AT+IPR 38400` to change the BR to `38400` in this example.
  2. `AT&W` to save the changes.
- This library **only** supports boards with an **AVR or SAM processor**.

## How to use
- Install our library using this [Arduino guide under ***Importing a .zip Library***](https://www.arduino.cc/en/Guide/Libraries#toc4)
	- This would copy our library folder (ArduinoMDot) to your Arduino IDE directory (eg: `C:\Users\{user}\Documents\Arduino\libraries`). This allows the Arduino application to access it's libraries.
	- You also need to install the **prerequisite** libraries such as [AltSoftSerial](https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html). 
- Create a new Arduino script or alternatively you could use one of the examples provided with the library which could be accessed from this path (eg: `C:\Users\{user}\Documents\Arduino\libraries`).
- Include the library in your new Arduino script: `#include <ArduinoMDot.h>`
- Before the `setup()` instruction, instantiate the mDot object: `ArduinoMDot mdot;`
- In the `setup()` function, initialise an instance of the mdot object by doing `mdot.begin();` (see [object oriented programming](https://en.wikipedia.org/wiki/Object-oriented_programming) wiki page for more information
- Now you can use the created instance's methods such as `mdot.join()` and `mdot.sendPairs()`, etc. 
  1. `mdot.join()` joins the LoRa network and returns a response code. If the response code is equal to 0 then the join has not been successful and you will need to run the `mdot.join()` command again. 
  2. Once the `mdot.join()` method is successful, the `mdot.networkSessionKey` method will return the relevant session key

### Explanation of Couple of Common Methods ##

### `join( )`
This method sends a join request to the LoRa server.

### `sendPairs( )`
This method accepts a comma separated key-value pair in the form of a `String` or `array of chars`
- example:
`Temp1:26.00,Temp2:22.70,RH%:59,Moral%:100`

### Wiring and debugging 
- Once you have uploaded the code to your dev board, you must remove the TX and RX wires (otherwise this will disrupt the mdot signals being sent to the LoRa network and you node will not connect) 
- Ensure that all your baudrates match. Everything must **send on** and **receive on** on 34100 (or the baudrate in the `begin()` statement)
- The mDot must be programmed with the Australian Firmware 
- The baudrate is effected by the speed of the On Board Crystal. If the microcontroller is programmed at 8 Mhz then the dev board performing the serial listening task for the debug serial must also have 8 Mhz 


The mdot object will post data to the serial on its own accord. An explanation of some likely debug responses is listed below: 
```
LaT:sp: {"L":66}            --> sp stands for *send pairs* 
LaT:sc: AT+TXDR?            --> sc stands for *send command* and TXDR stands for transmit data rate (the ? is asking for a response) 
LaT:sc: AT+SEND 11{"L":66}  --> the pairs of data is sent 
```
For more information on AT commands, please see the Multech [AT Command Getting Started Guide](http://www.multitech.net/developer/software/mdot-software/at-command-firmware-user-guide/) and the [DOT Series AT Command Reference Guide](http://www.multitech.com/documents/publications/manuals/s000643.pdf).

### Register your Node in AltoView
- Once you have identified the LoRa node has successful connected to the LoRa network and is sending data, navigate to [AltoView](http://www.altoview.com/) (create a login if you don't already have one) 
- On the left hand side menu, select **My Nodes** and click **Register a Node** 
- Enter the DevEUI ([Development Extended Unique Identifier](https://en.wikipedia.org/wiki/MAC_address)) which can be found on the mDot node in small print (look for a 16 value long ID with numbers and letters in it, eg: 00 80 00 00 00 00 AA 66) 

#### NB when registering node in AltoView 
- Although the DevEUI is written in *uppercase* on most mDot modules, Altoview requirers the DevEUI in ***LOWERCASE*** 
- Once the node details have been entered and you have hit the `register` button, the node will have either an `online` or `offline` status. Please note that this status **does not** mean that the node is powered on and connected to the LoRa network. Instead: 
  - `online` --> the node was connected and sending data in the past. In other words, there is a history of the node having sent data
  - `offline` --> the node has never sent data. In other words, AltoView has never seen this nodes DevEUI before 


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
