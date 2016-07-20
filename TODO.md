# TODO
## Version 1 Goal
- [x] Get and Set ADR.
- [x] Default to set Data Rate to [?] and turn ADR on
- [ ] Check mDot Data rate before send.
- [ ] Send multiple fragments based on Data Rate.
- [ ] Check mDot for response on 38400, 115200, 9600 and some others. If mDot detected not to have a baud rate of u32BaudRate set it to u32BaudRate.
- [ ] Public Member to indicate last get/set result.
   * What if we do multiple get/sets in a row, and all fail expect the last?
- [ ] Check _pairToJson() works for the following cases:
   * Pairs expand out leaving ample space to finish JSON with }\0
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b : d e f , h i             :  V
	 + Simply add }\0 to end
   * Pairs expand out leaving just enough space to finish JSON with }\0
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b : d e f , h i             U  V  :  X
	 + Simply add }\0 to end
   * No comma found
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b c d e f g h i             U  V  W  X  Y  Z
	 + Cycle all the way back to beginning and add {}\0
   * Comma found in last two 
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b c d e f g h i             U  V  W  X  ,  Z
	 - [ ] If end is reached, always delete last two before looking for comma. Mention this explicitly in comments.
   * Comma found with ample space
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b c d e f g h i             ,  V  W  X  Y  Z
   * Comma found with just enough space
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b c d e f g h i             U  V  W  ,  Y  Z
- [ ] Check what Data Rate should default to.
	 
## Future Version Goals
- [ ] Overload send() function so it can also accept a char* and length. For when we don't want to read until null.
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
- [ ] strlen is used on pairs[] which may not be null terminated.
   * If a user passes a string bigger than the buffer. The program reads in that string up to the size of the buffer, in that case the last character will be what's in the string at that point (not nessicarly a null).
   * The user could also pass a non-null terminated char array of any lenght and also cause the same fault.
- [ ] Handle JSON buffer smaller than 2?
- [ ] Max end time rollover bug
- [ ] unsigned int, and similar needs to be explicitly an uintx_t
- [ ] Server expects a new join request every 5 days
   - [ ] long term timer to monitor, time since last join
   - [ ] Check timer before send(), and join if time condition met
- [ ] Always save session after join.
- [ ] Look at overloading some of the setters