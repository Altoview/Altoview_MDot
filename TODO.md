[x] Init mDot by default
   [ ] Set Data Rate and turn ADR on
[ ] Default library baud rate 38400
[ ] Check for response on 38400, 115200, 9600 and some others. If not 38400 set to 38400.
[ ] Check ADR before send
[ ] Overload send() function so it can also accept a char* and length
[ ] Memeber to indicate last get/set result
[ ] Ensure pairs[] and json[] are released after they're used. They are big chunks of memory on the stack.
[ ] Handle a fragment failing to send?
   [ ] Skip the rest?
   [ ] Try every one?
   [ ] Give a general failure or a specific one?
[ ] Handle failing setting/getting one setting/getting
   [ ] Skip the rest?
   [ ] Try every one?
   [ ] Give a general failure or a specific one?
[ ] Check we're not using sizeof on pointers
   * Size of a pointer is 2bytes, not the size of the array it points to.
[ ] Handle JSON buffer smaller than 2?
[ ] Check when we're using strlen it's on a null terminated array.
[ ] Check pairs to JSON works for the following cases
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
	 [ ] If end is reached, always delete last two before looking for comma. Mention this explicity in comments.
   * Comma found with ample space
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b c d e f g h i             ,  V  W  X  Y  Z
   * Comma found with just enough space
     + 0 1 2 3 4 5 6 7 8 9 ...... .... 95 96 97 98 99 100
     + { a b c d e f g h i             U  V  W  ,  Y  Z
[ ] Generally check header comments for spelling and incorect discription
[ ] Improve header comments, esp. return explinations
   [ ] e.g. begin header "Anything not a valid option for serial will be set to 0"
[ ] Time outs always throw -1
[ ] Errors always negative, warnings and success are always positive
[ ] Max end time rollover bug
[ ] If the command can't be found in _response return null, explain this will be the case in the header
[ ] unsigned int, and similar needs to be explicity an uintx_t
[ ] Server expects a new join request every 5 days
   [ ] long term timer to monitor, time since last join
   [ ] Check timer before send(), and join if time condidtion met
[ ] always save session after join
[ ] Check if user passes too big string of pairs? Bigger than the array.
[ ] Always initilise response to error, before stepping though function.
[x] use memcpy more in createJson().
   [x] Don't forget to increment pointer
[ ] json if/else can be switch
[ ] for/if can be merged, put if in for loop conditional statement