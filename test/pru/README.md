### How to test the source file running in PRU ###
## Problem ##
Software development in PRU is limited by the debugging tools available in a single processor development. We can't use print or view all memory register in an conmercial IDE or use any JTAG related debugging tools. 
## Solution ##
Here we proposed two solutions to tackle this problem. The first solution is to test the code in userspace first as both PRU code and user space code are C99 compatible. 

 1. Make sure rx.c will general raw samples from the ADC. Meanwhile, the tx.c needs to send msgs continuously.
 2. Let's assume the msg.log contains the raw samples from the ADC. Before you apply the preprocess script. You'll need to hexdump it
```
hexdump -d msg.log >> msg.log
```
 3. Now you run the script to convert the msg.log file to a sequency of 0's and 1's for the source file to decode. This is when you'll have the test.txt file used by the test.c file.
 4. Compile test.c 
```
gcc test.c -o test
```
 5. Run the executable test.
```
test
```
 6. Now you will see the the decoded message in the terminal. You can insertprintf in all places you want for debugging purpose. 
 7. Enjoy!!!  

The second solution is to use one PRU as software development processor and the other PRU as the debugging processor. The purple board provideds an onboard RGB LED. As the two PRUs have shared memory. We can allocate a global pointer for these two PRU, and modify the value for this pointer in one PRU and check whether the value has been updated in the debugging PRU. If the value is updated, we can then blink the LED as the notice to the user such that the user knows exactly which code block has trigger the LED. 

PRU0
```
uint32_t *clockPointer = (uint32_t *)0x00010000;

while( *clockPointer == SOME INTEGER)
{
 	blink the led;
}
```

PRU1
```
uint32_t *clockPointer = (uint32_t *)0x00010000;

void blink(void)
{
	*clockPointer = SOME INTEGER;
}

```



