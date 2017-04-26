# TODO
## Version 1 Goal
- [x] Get and Set ADR.
- [x] Default to set Data Rate to [?] and turn ADR on
- [x] Check mDot Data rate before send.
- [x] Send multiple fragments based on Data Rate.
- [x] Send correct number of fragments based on Data Rate.
- [x] strlen is used on pairs[] which may not be null terminated.
   * If a user passes a string bigger than the buffer. The program reads in that string up to the size of the buffer, in that case the last character will be what's in the string at that point (not necessarily a null).
   * The user could also pass a non-null terminated char array of any length and also cause the same fault.
- [x] Always save session after join.
- [x] Server expects a new join request every 5 days
   - [x] long term timer to monitor, time since last join
   - [x] Check timer before send(), and join if time condition met
- [ ] Check mDot for response on 38400, 115200, 9600 and some others. If mDot detected not to have a baud rate of u32BaudRate set it to u32BaudRate.
- [ ] Public Member to indicate last get/set result.
   * What if we do multiple get/sets in a row, and all fail except the last?
- [x] Check pairToJson() works for the following cases:
   - [x] Pairs expand out leaving ample space to finish JSON with }\0
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b : d e f , h i             :  V
     + Simply add }\0 to end
   - [x] Pairs expand out leaving just enough space to finish JSON with }\0
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b : d e f , h i             U  V  :  X
     + Simply add }\0 to end
   - [x] No comma found
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b c d e f g h i             U  V  W  X  Y  Z
     + Cycle all the way back to beginning and add {}\0
   - [x] Comma found in last two
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b c d e f g h i             U  V  W  X  ,  Z
   - [x] Comma found with ample space
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b c d e f g h i             ,  V  W  X  Y  Z
   - [x] Comma found with just enough space
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b c d e f g h i             U  V  W  X  ,  Z
   - [x] If end is reached, always delete last two before looking for comma. Mention this explicitly in comments.
- [ ] Check what Data Rate should default to.
- [ ] Amend code in supplied arduino example scripts to include AT response codes --> for debugging and resolving of connection issues 
- [ ] LoRat::restoreLoraSession is commented out in library but is being used in arduino example scripts --> results in compilation error for supplied examples

## Future Version Goals
- [x] Overload send() function so it can also accept a char* and length. For when we don't want to read until null.
- [ ] Cover **SAM** series. **AVR** series are already covered.
- [ ] Check for wider range of responses from mDot.
- [ ] Change txPutter to number of bytes, not number of fragments. Or change name to be more meaningful.
- [ ] Ensure pairs[] and json[] are released after they're used. They are big chunks of memory on the stack.
   - [ ] Consider other ways to handle pairs[] and json[]
- [ ] Handle a fragment failing to send?
   - [ ] Skip the rest?
   - [ ] Try every one? Give the last result? **This is the current state**
   - [ ] Give a general failure or a specific one?
- [ ] Handle failing setting/getting one setting/getting
   - [ ] Skip the rest?
   - [ ] Try every one? Give the last result? **This is the current state**
   - [ ] Give a general failure or a specific one?
- [ ] Check for invalid characters user could pass to pairs?
- [ ] Check pairs is a valid entry?
- [ ] Handle JSON buffer smaller than 2?
- [ ] Max end time rollover bug
- [x] unsigned int, and similar needs to be explicitly an uintx_t
- [ ] Look at overloading some of the setters
- [ ] Bug fix, seems to be a null in any long response gets
