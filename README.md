# Project 2

- Name: Amanda Tuttle
- Email: amandatuttle631@u.boisestate.edu
- Class: CS525-001

## Known Bugs or Issues

In terms of functionality there are not any bugs that I know of, but the code coverage is not 100%. 

## Experience

This project was a lot. I understood the protocol, the state machines, and the function the code was supposed to perform, but I initially had a very difficult time breaking down how that functionality could be turned into actual functions. I eventually used AI to help come up with the layer split described in the design section, but it made a lot of stupid decisions that I knew would be very difficult to test for later me (e.g., it wanted all of the the make packet functions to return void). I also faced my usual share of technical issues while I was working such as my virtual machine deciding it no longer wanted to connect to the internet (it said it was connected, it lied), and the fact that apparently the vm was running python 3.8, which is seemingly old enough to cause the relay (aka the single piece of code I could count on to actually work) to not run at all. 

My strategy for the code itself was to write a function, write all the unit tests for that function, and if all the tests all passed I could move on to the next one. This seemed to be working well all the way up until I finished lab.c. I had 208 tests, every last one was passing, and for reasons unbeknownst to me, the relay was failing. I was so incredibly close to giving up, with not enough time and no idea how to start debugging, since all of my theoretical tests were passing, when I accidentally gave myself a massive hint when running ./build/release/myapp send -s me -w 16 -l 0.1 -c 0.05 127.0.0.1 in.bin with a file size of only two packets ended with a successful transfer; I had run the same parameters on the same file before and it failed, and I noticed that this time the sender had no dropped/corrupted packets but the receiver did, which immediately told me the issue was related to the sender resending packets. The problem ended up being that in runSender, I was comparing the timer status to the wrong value--senderTimerDue returned 0 if the timer was active, but runSender was checking if it was above or below 0. After fixing this, I felt for a moment that this must be what it's like to be a sports fan as I was at the edge of my seat waiting for the relay to run. Minus the sports. Though I did get up out of my chair and take a lap after it worked.

The one part of this project I did unfortunately have to leave unfinished was the code coverage. I started at 55% and after probably an hour of reviewing the gaps I only made it to 59%, and I realized I just wouldn't have the time to dedicate to bring it to 100. I definitely underestimated the amount of time I would need for the portion of the project after lab.c was finished, between debugging the relay not working and updating code coverage, since those steps in project 1 didn't take me long at all in comparison. Although 59% is painfully low, I almost turned in a project that didn't work *and* had low coverage, so in comparison this is much better!

## Design

The functions in this project are split into three layers: packets, the Go-Back-N state machines. and the I/O. The split is as follows:
- Packet functions - packetChecksum, encodePacket, decodePacket, makeDataPacket, makeAckPacket, makeFinPacket
- State machines - senderInit, senderSendData, senderHandleAck, senderHandleTimeout, senderTimerDue, senderSendFin, senderHandleFinAck, senderFinished, receiverInit, receiverHandleData, receiverHandleFin, receiverTimerExpired
- I/O - getTimeMs, resolveAddr, createSocket, registerReceiver, registerSender, runSender, runReceiver, printUsage

This is done for purposes of testing, and enables the packet functions and state machines to be tested separately from I/O and without a live server. 


## Results

| Window	| Loss	| Corrupt	| Dup | Time s | Throughput KiB/s |
|-------|-------|-----------|----|-------|------------|
| 1	| 0	| 0 |	0 | 107 | 9.3 | 
| 16 |	0	| 0	| 0 | 8.8 | 113.6 |
| 1	| 0.05	| 0	| 0 | 131.5 | 7.6 |
| 16	| 0.05	| 0	| 0 | 26.4 | 37.9 |

In the window 1, no loss run, the sender saw an RTT of 104.4 ms. My assumption is that the extra 4 ms is a result of overhead from the program; especially with a window size of 1, the sender has to wait until the ack from the receiver is received and fully processed before even beginning to send the next packet. The speed up from window 1 to window 16 is by a factor of about 12, which makes sense since each packet no longer has to be processed one at a time fully before continuing, but there is still a non negligible amount of time for processing. Compared to the no loss time, 5% loss slows down the window 16 so much more than the window 1 because every time a packet times out, the sender must resend everything from base (the packet that timed out) to n-1, which in the case of window 1, is only 1 packet, but in the case of window 16, is all 16 packets. With a 5% loss, window 16 is still much faster than window 1, but by a much lower factor than when compared to no loss. 
