# Submission Report

- Submission generated at 10/10/2026 at 03:46:58

- Machine info: Linux runnervmmprz5 6.17.0-1022-azure #22-Ubuntu SMP Mon Jul 27 17:24:03 UTC 2026 x86_64 x86_64 x86_64 GNU/Linux

## Note to Students

Please read this report carefully before submission.
Ensure that all sections are complete and accurate.
Look for any errors in the build or test outputs.
If you find any issues, correct them before submitting.
Post any questions on the class discussion board for help.


---

## README

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

---


## Build Output

This section was generated by running `make all` in the project root directory.

```bash
make[1]: Entering directory '/home/runner/work/cs525-p2/cs525-p2'
mkdir -p build/debug
cc -g -O0 -DDEBUG -fno-omit-frame-pointer -fsanitize=address -c src/lab.c -o build/debug/lab.c.o
mkdir -p build/debug
cc -g -O0 -DDEBUG -fno-omit-frame-pointer -fsanitize=address -c src/main.c -o build/debug/main.c.o
cc -g -O0 -DDEBUG -fno-omit-frame-pointer -fsanitize=address build/debug/lab.c.o build/debug/main.c.o -o build/debug/myapp_d -fsanitize=address
make[1]: Leaving directory '/home/runner/work/cs525-p2/cs525-p2'
make[1]: Entering directory '/home/runner/work/cs525-p2/cs525-p2'
mkdir -p build/release
cc -Wall -Wextra -O2 -fPIE -MMD -MP -Wformat -Wformat=2 -Wconversion -Wsign-conversion -Wimplicit-fallthrough -fstack-protector-strong -Werror=format-security -Werror=implicit -Werror=incompatible-pointer-types -Werror=int-conversion -c src/lab.c -o build/release/lab.c.o
mkdir -p build/release
cc -Wall -Wextra -O2 -fPIE -MMD -MP -Wformat -Wformat=2 -Wconversion -Wsign-conversion -Wimplicit-fallthrough -fstack-protector-strong -Werror=format-security -Werror=implicit -Werror=incompatible-pointer-types -Werror=int-conversion -c src/main.c -o build/release/main.c.o
cc -Wall -Wextra -O2 -fPIE -MMD -MP -Wformat -Wformat=2 -Wconversion -Wsign-conversion -Wimplicit-fallthrough -fstack-protector-strong -Werror=format-security -Werror=implicit -Werror=incompatible-pointer-types -Werror=int-conversion build/release/lab.c.o build/release/main.c.o -o build/release/myapp 
make[1]: Leaving directory '/home/runner/work/cs525-p2/cs525-p2'
make[1]: Entering directory '/home/runner/work/cs525-p2/cs525-p2'
mkdir -p build/tests
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage -c src/lab.c -o build/tests/lab.c.o
mkdir -p build/tests
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage -c src/main.c -o build/tests/main.c.o
mkdir -p build/tests/
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage -c tests/lab-test.c -o build/tests/lab-test.c.o
mkdir -p build/tests/harness/
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage -c tests/harness/unity.c -o build/tests/harness/unity.c.o
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage build/tests/lab.c.o build/tests/main.c.o build/tests/lab-test.c.o build/tests/harness/unity.c.o -o build/tests/myapp_t -fprofile-arcs -ftest-coverage
make[1]: Leaving directory '/home/runner/work/cs525-p2/cs525-p2'
make[1]: Entering directory '/home/runner/work/cs525-p2/cs525-p2'
mkdir -p build/debug-test
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address -c src/lab.c -o build/debug-test/lab.c.o
mkdir -p build/debug-test
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address -c src/main.c -o build/debug-test/main.c.o
mkdir -p build/debug-test/
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address -c tests/lab-test.c -o build/debug-test/lab-test.c.o
mkdir -p build/debug-test/harness/
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address -c tests/harness/unity.c -o build/debug-test/harness/unity.c.o
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address build/debug-test/lab.c.o build/debug-test/main.c.o build/debug-test/lab-test.c.o build/debug-test/harness/unity.c.o -o build/debug-test/myapp_td -fsanitize=address
make[1]: Leaving directory '/home/runner/work/cs525-p2/cs525-p2'
Builds completed. You can run the application with: ./build/release/myapp
You can run the debug build with: ./build/debug/myapp_d
You can run the test build with: ./build/tests/myapp_t
You can run the debug-test build with: ./build/debug-test/myapp_td
```

---

## Coverage Report

This section was generated by running `make report` in the project root directory.

```bash
tests/lab-test.c:4816:test_packetChecksum_empty:PASS
tests/lab-test.c:4817:test_packetChecksum_singleByte:PASS
tests/lab-test.c:4818:test_packetChecksum_twoBytes:PASS
tests/lab-test.c:4819:test_packetChecksum_fourBytes:PASS
tests/lab-test.c:4820:test_packetChecksum_oddLength:PASS
tests/lab-test.c:4821:test_packetChecksum_oddLength:PASS
tests/lab-test.c:4823:test_makeDataPacket_validPacket:PASS
tests/lab-test.c:4824:test_makeDataPacket_emptyPayload:PASS
tests/lab-test.c:4825:test_makeDataPacket_maxPayload:PASS
tests/lab-test.c:4826:test_makeDataPacket_nullPacket:PASS
tests/lab-test.c:4827:test_makeDataPacket_nullDataWithNonzeroLength:PASS
tests/lab-test.c:4828:test_makeDataPacket_payloadTooLarge:PASS
tests/lab-test.c:4829:test_makeDataPacket_zeroLengthWithNonNullData:PASS
tests/lab-test.c:4831:test_makeAckPacket_validPacket:PASS
tests/lab-test.c:4832:test_makeAckPacket_zeroSequenceNumber:PASS
tests/lab-test.c:4833:test_makeAckPacket_maxSequenceNumber:PASS
tests/lab-test.c:4834:test_makeAckPacket_nullPacket:PASS
tests/lab-test.c:4836:test_makeFinPacket_validPacket:PASS
tests/lab-test.c:4837:test_makeFinPacket_zeroSequenceNumber:PASS
tests/lab-test.c:4838:test_makeFinPacket_maxSequenceNumber:PASS
tests/lab-test.c:4839:test_makeFinPacket_nullPacket:PASS
tests/lab-test.c:4841:test_encodePacket_dataPacket:PASS
tests/lab-test.c:4842:test_encodePacket_ackPacket:PASS
tests/lab-test.c:4843:test_encodePacket_finPacket:PASS
tests/lab-test.c:4844:test_encodePacket_emptyDataPacket:PASS
tests/lab-test.c:4845:test_encodePacket_maxPayload:PASS
tests/lab-test.c:4846:test_encodePacket_nullPacket:PASS
tests/lab-test.c:4847:test_encodePacket_nullBuffer:PASS
tests/lab-test.c:4848:test_encodePacket_nullPacketSize:PASS
tests/lab-test.c:4849:test_encodePacket_bufferTooSmall:PASS
tests/lab-test.c:4850:test_encodePacket_invalidType:PASS
tests/lab-test.c:4851:test_encodePacket_ackWithPayload:PASS
tests/lab-test.c:4852:test_encodePacket_finWithPayload:PASS
tests/lab-test.c:4853:test_encodePacket_payloadTooLarge:PASS
tests/lab-test.c:4855:test_decodePacket_validDataPacket:PASS
tests/lab-test.c:4856:test_decodePacket_validAckPacket:PASS
tests/lab-test.c:4857:test_decodePacket_validFinPacket:PASS
tests/lab-test.c:4858:test_decodePacket_emptyDataPacket:PASS
tests/lab-test.c:4859:test_decodePacket_maxPayload:PASS
tests/lab-test.c:4860:test_decodePacket_nullBuffer:PASS
tests/lab-test.c:4861:test_decodePacket_nullPacket:PASS
tests/lab-test.c:4862:test_decodePacket_tooShort:PASS
tests/lab-test.c:4863:test_decodePacket_tooLarge:PASS
tests/lab-test.c:4864:test_decodePacket_invalidType:PASS
tests/lab-test.c:4865:test_decodePacket_nonzeroReservedByte:PASS
tests/lab-test.c:4866:test_decodePacket_lengthMismatch:PASS
tests/lab-test.c:4867:test_decodePacket_corruptedPayload:PASS
tests/lab-test.c:4868:test_decodePacket_corruptedSequenceNumber:PASS
tests/lab-test.c:4869:test_decodePacket_ackWithPayload:PASS
tests/lab-test.c:4870:test_decodePacket_finWithPayload:PASS
tests/lab-test.c:4872:test_senderInit_validArguments:PASS
tests/lab-test.c:4873:test_senderInit_minimumWindow:PASS
tests/lab-test.c:4874:test_senderInit_maximumWindow:PASS
tests/lab-test.c:4875:test_senderInit_nullPointer:PASS
tests/lab-test.c:4876:test_senderInit_zeroWindow:PASS
tests/lab-test.c:4877:test_senderInit_windowTooLarge:PASS
tests/lab-test.c:4878:test_senderInit_zeroTimeout:PASS
tests/lab-test.c:4879:test_senderInit_resetsState:PASS
tests/lab-test.c:4881:test_senderSendData_firstPacket:PASS
tests/lab-test.c:4882:test_senderSendData_storesPacket:PASS
tests/lab-test.c:4883:test_senderSendData_multiplePackets:PASS
tests/lab-test.c:4884:test_senderSendData_nullSender:PASS
tests/lab-test.c:4885:test_senderSendData_nullData:PASS
tests/lab-test.c:4886:test_senderSendData_nullPacket:PASS
tests/lab-test.c:4887:test_senderSendData_zeroLength:PASS
tests/lab-test.c:4888:test_senderSendData_payloadTooLarge:PASS
tests/lab-test.c:4889:test_senderSendData_fullWindow:PASS
tests/lab-test.c:4890:test_senderSendData_afterFinSent:PASS
tests/lab-test.c:4891:test_senderSendData_afterFinAcked:PASS
tests/lab-test.c:4893:test_senderHandleAck_advancesBase:PASS
tests/lab-test.c:4894:test_senderHandleAck_duplicateAck:PASS
tests/lab-test.c:4895:test_senderHandleAck_staleAck:PASS
tests/lab-test.c:4896:test_senderHandleAck_ackBeyondNext:PASS
tests/lab-test.c:4897:test_senderHandleAck_allDataAcknowledged:PASS
tests/lab-test.c:4898:test_senderHandleAck_noOutstandingData:PASS
tests/lab-test.c:4899:test_senderHandleAck_nullSender:PASS
tests/lab-test.c:4900:test_senderHandleAck_resetsTimeoutCount:PASS
tests/lab-test.c:4902:test_senderHandleTimeout_retransmitsAllOutstanding:PASS
tests/lab-test.c:4903:test_senderHandleTimeout_retransmitsUnacknowledgedOnly:PASS
tests/lab-test.c:4904:test_senderHandleTimeout_restartsTimer:PASS
tests/lab-test.c:4905:test_senderHandleTimeout_incrementsTimeoutCount:PASS
tests/lab-test.c:4906:test_senderHandleTimeout_timerNotExpired:PASS
tests/lab-test.c:4907:test_senderHandleTimeout_timerNotRunning:PASS
tests/lab-test.c:4908:test_senderHandleTimeout_nullSender:PASS
tests/lab-test.c:4909:test_senderHandleTimeout_nullPackets:PASS
tests/lab-test.c:4910:test_senderHandleTimeout_nullCount:PASS
tests/lab-test.c:4911:test_senderHandleTimeout_timeWentBackward:PASS
tests/lab-test.c:4912:test_senderHandleTimeout_exactExpiration:PASS
tests/lab-test.c:4913:test_senderHandleTimeout_noOutstandingPackets:PASS
tests/lab-test.c:4914:test_senderHandleTimeout_exceedsConfiguredWindow:PASS
tests/lab-test.c:4917:test_getTimeMs_returns_nonzero:PASS
tests/lab-test.c:4918:test_getTimeMs_is_monotonic:PASS
tests/lab-test.c:4919:test_getTimeMs_does_not_decrease_over_time:PASS
tests/lab-test.c:4920:test_getTimeMs_reasonable_timestamp:PASS
tests/lab-test.c:4922:test_senderTimerDue_null_sender:PASS
tests/lab-test.c:4923:test_senderTimerDue_null_remaining:PASS
tests/lab-test.c:4924:test_senderTimerDue_timer_not_running:PASS
tests/lab-test.c:4925:test_senderTimerDue_full_timeout_remaining:PASS
tests/lab-test.c:4926:test_senderTimerDue_partial_timeout_remaining:PASS
tests/lab-test.c:4927:test_senderTimerDue_timer_expired:PASS
tests/lab-test.c:4928:test_senderTimerDue_timer_expired_long_ago:PASS
tests/lab-test.c:4929:test_senderTimerDue_time_before_timer_start:PASS
tests/lab-test.c:4930:test_senderTimerDue_one_ms_before_expiration:PASS
tests/lab-test.c:4931:test_senderTimerDue_zero_timeout:PASS
tests/lab-test.c:4933:test_senderSendFin_null_sender:PASS
tests/lab-test.c:4934:test_senderSendFin_null_packet:PASS
tests/lab-test.c:4935:test_senderSendFin_unacknowledged_data:PASS
tests/lab-test.c:4936:test_senderSendFin_success:PASS
tests/lab-test.c:4937:test_senderSendFin_empty_file:PASS
tests/lab-test.c:4938:test_senderSendFin_already_sent:PASS
tests/lab-test.c:4939:test_senderSendFin_does_not_modify_state_on_failure:PASS
tests/lab-test.c:4941:test_senderHandleFinAck_null_sender:PASS
tests/lab-test.c:4942:test_senderHandleFinAck_fin_not_sent:PASS
tests/lab-test.c:4943:test_senderHandleFinAck_already_acknowledged:PASS
tests/lab-test.c:4944:test_senderHandleFinAck_wrong_ack_too_small:PASS
tests/lab-test.c:4945:test_senderHandleFinAck_wrong_ack_too_large:PASS
tests/lab-test.c:4946:test_senderHandleFinAck_success:PASS
tests/lab-test.c:4947:test_senderHandleFinAck_empty_file:PASS
tests/lab-test.c:4948:test_senderHandleFinAck_timer_already_stopped:PASS
tests/lab-test.c:4950:test_senderFinished_null_sender:PASS
tests/lab-test.c:4951:test_senderFinished_no_packets_sent:PASS
tests/lab-test.c:4952:test_senderFinished_data_not_fully_acknowledged:PASS
tests/lab-test.c:4953:test_senderFinished_fin_not_sent:PASS
tests/lab-test.c:4954:test_senderFinished_fin_not_acknowledged:PASS
tests/lab-test.c:4955:test_senderFinished_success:PASS
tests/lab-test.c:4956:test_senderFinished_empty_file_complete:PASS
tests/lab-test.c:4957:test_senderFinished_data_acknowledged_but_fin_not_sent:PASS
tests/lab-test.c:4959:test_receiverInit_null_receiver:PASS
tests/lab-test.c:4960:test_receiverInit_success:PASS
tests/lab-test.c:4961:test_receiverInit_resets_existing_state:PASS
tests/lab-test.c:4962:test_receiverInit_sets_activity_timestamp:PASS
tests/lab-test.c:4964:test_receiverHandleData_expeReceiver registration failed: receiver required
Receiver registration failed: no valid OK response after 5 attempts
Sender registration failed: receiver required
ctedPacket:PASS
tests/lab-test.c:4965:test_receiverHandleData_multipleInOrderPackets:PASS
tests/lab-test.c:4966:test_receiverHandleData_duplicatePacket:PASS
tests/lab-test.c:4967:test_receiverHandleData_outOfOrderPacket:PASS
tests/lab-test.c:4968:test_receiverHandleData_missingPacket:PASS
tests/lab-test.c:4969:test_receiverHandleData_zeroLengthPayload:PASS
tests/lab-test.c:4970:test_receiverHandleData_maxPayload:PASS
tests/lab-test.c:4971:test_receiverHandleData_nullReceiver:PASS
tests/lab-test.c:4972:test_receiverHandleData_nullPacket:PASS
tests/lab-test.c:4973:test_receiverHandleData_nullAck:PASS
tests/lab-test.c:4974:test_receiverHandleData_nonDataPacket:PASS
tests/lab-test.c:4975:test_receiverHandleData_oversizedPayload:PASS
tests/lab-test.c:4976:test_receiverHandleData_receiverFinished:PASS
tests/lab-test.c:4977:test_receiverHandleData_repeatedOutOfOrderPackets:PASS
tests/lab-test.c:4979:test_receiverHandleFin_nullReceiver:PASS
tests/lab-test.c:4980:test_receiverHandleFin_nullPacket:PASS
tests/lab-test.c:4981:test_receiverHandleFin_nullAck:PASS
tests/lab-test.c:4982:test_receiverHandleFin_nonFinPacket:PASS
tests/lab-test.c:4983:test_receiverHandleFin_nonzeroPayload:PASS
tests/lab-test.c:4984:test_receiverHandleFin_expectedSequence:PASS
tests/lab-test.c:4985:test_receiverHandleFin_emptyFile:PASS
tests/lab-test.c:4986:test_receiverHandleFin_outOfOrderSequence:PASS
tests/lab-test.c:4987:test_receiverHandleFin_earlyFin:PASS
tests/lab-test.c:4988:test_receiverHandleFin_duplicateFin:PASS
tests/lab-test.c:4989:test_receiverHandleFin_unexpectedFinAfterCompletion:PASS
tests/lab-test.c:4990:test_receiverHandleFin_activityTimestamp:PASS
tests/lab-test.c:4992:test_receiverTimerExpired_nullReceiver:PASS
tests/lab-test.c:4993:test_receiverTimerExpired_noElapsedTime:PASS
tests/lab-test.c:4994:test_receiverTimerExpired_lessThanTimeout:PASS
tests/lab-test.c:4995:test_receiverTimerExpired_exactlyAtTimeout:PASS
tests/lab-test.c:4996:test_receiverTimerExpired_pastTimeout:PASS
tests/lab-test.c:4997:test_receiverTimerExpired_timeBeforeLastActivity:PASS
tests/lab-test.c:4998:test_receiverTimerExpired_zeroLastActivity:PASS
tests/lab-test.c:4999:test_receiverTimerExpired_largeTimestamps:PASS
tests/lab-test.c:5000:test_receiverTimerExpired_doesNotModifyState:PASS
tests/lab-test.c:5002:test_resolveAddr_validIPv4:PASS
tests/lab-test.c:5003:test_resolveAddr_validHostname:PASS
tests/lab-test.c:5004:test_resolveAddr_invalidHostname:PASS
tests/lab-test.c:5005:test_resolveAddr_nullHost:PASS
tests/lab-test.c:5006:test_resolveAddr_nullPort:PASS
tests/lab-test.c:5007:test_resolveAddr_nullResults:PASS
tests/lab-test.c:5008:test_resolveAddr_failureClearsResults:PASS
tests/lab-test.c:5009:test_resolveAddr_numericService:PASS
tests/lab-test.c:5011:test_createSocket_validUDPIPv4:PASS
tests/lab-test.c:5012:test_createSocket_validUDPIPv6:PASS
tests/lab-test.c:5013:test_createSocket_nullAddress:PASS
tests/lab-test.c:5014:test_createSocket_invalidFamily:PASS
tests/lab-test.c:5015:test_createSocket_invalidSocketType:PASS
tests/lab-test.c:5016:test_createSocket_invalidProtocol:PASS
tests/lab-test.c:5018:test_registerReceiver_success:PASS
tests/lab-test.c:5019:test_registerReceiver_relayError:PASS
tests/lab-test.c:5020:test_registerReceiver_nullRelay:PASS
tests/lab-test.c:5021:test_registerReceiver_nullSession:PASS
tests/lab-test.c:5022:test_registerReceiver_invalidSessionCharacter:PASS
tests/lab-test.c:5023:test_registerReceiver_sessionTooLong:PASS
tests/lab-test.c:5024:test_registerReceiver_invalidSocket:PASS
tests/lab-test.c:5025:test_registerReceiver_ipv6_address_check:PASS
tests/lab-test.c:5026:test_registerReceiver_fails_after_five_attempts:PASS
tests/lab-test.c:5028:test_registerSender_success:PASS
tests/lab-test.c:5029:test_registerSender_relayError:PASS
tests/lab-test.c:5030:test_registerSender_nullRelay:PASS
tests/lab-test.c:5031:test_registerSender_nullSession:PASS
tests/lab-test.c:5032:test_registerSender_invalidSession:PASS
tests/lab-test.c:5033:test_registerSender_sessionTooLong:PASS
tests/lab-test.c:5034:test_registerSender_negativeLoss:PASS
tests/Sender registration failed after 5 attempts
runSender: invalid argument
runSender: invalid argument
runSender: invalid window or timeout
runSender: invalid window or timeout
runSender: invalid window or timeout
fopen: No such file or directory
runSender: failed to resolve relay
Sender registration failed: test rejection
runSender: relay registration failed
lab-test.c:5035:test_registerSender_corruptTooHigh:PASS
tests/lab-test.c:5036:test_registerSender_dupTooHigh:PASS
tests/lab-test.c:5037:test_registerSender_invalidSocket:PASS
tests/lab-test.c:5038:test_registerSender_ipv6_success:PASS
tests/lab-test.c:5039:test_registerSender_wrongRelayAddress:PASS
tests/lab-test.c:5041:test_runSender_nullRelay_returnsFailure:PASS
tests/lab-test.c:5042:test_runSender_nullFilename_returnsFailure:PASS
tests/lab-test.c:5043:test_runSender_zeroWindow_returnsFailure:PASS
tests/lab-test.c:5044:test_runSender_windowAboveMaximum_returnsFailure:PASS
tests/lab-test.c:5045:test_runSender_zeroTimeout_returnsFailure:PASS
tests/lab-test.c:5046:test_runSender_missingFile_returnsFailure:PASS
tests/lab-test.c:5047:test_runSender_invalidPort_returnsFailure:PASS
tests/lab-test.c:5048:test_runSender_relayRejectsRegistration_returnsFailure:PASS
tests/lab-test.c:5050:test_runReceiver_validDataAndFin:PASS
tests/lab-test.c:5051:test_runReceiver_emptyFile:PASS
tests/lab-test.c:5052:test_runReceiver_discardsCorruptedPacket:PASS
tests/lab-test.c:5053:test_runReceiver_outOfOrderData:PASS
tests/lab-test.c:5055:test_completeTransfer_withLossCorruptionAndDuplication:PASS

-----------------------
214 Tests 0 Failures 0 Ignored 
OK
./build/tests/myapp_t
tests/lab-test.c:4816:test_packetChecksum_empty:PASS
tests/lab-test.c:4817:test_packetChecksum_singleByte:PASS
tests/lab-test.c:4818:test_packetChecksum_twoBytes:PASS
tests/lab-test.c:4819:test_packetChecksum_fourBytes:PASS
tests/lab-test.c:4820:test_packetChecksum_oddLength:PASS
tests/lab-test.c:4821:test_packetChecksum_oddLength:PASS
tests/lab-test.c:4823:test_makeDataPacket_validPacket:PASS
tests/lab-test.c:4824:test_makeDataPacket_emptyPayload:PASS
tests/lab-test.c:4825:test_makeDataPacket_maxPayload:PASS
tests/lab-test.c:4826:test_makeDataPacket_nullPacket:PASS
tests/lab-test.c:4827:test_makeDataPacket_nullDataWithNonzeroLength:PASS
tests/lab-test.c:4828:test_makeDataPacket_payloadTooLarge:PASS
tests/lab-test.c:4829:test_makeDataPacket_zeroLengthWithNonNullData:PASS
tests/lab-test.c:4831:test_makeAckPacket_validPacket:PASS
tests/lab-test.c:4832:test_makeAckPacket_zeroSequenceNumber:PASS
tests/lab-test.c:4833:test_makeAckPacket_maxSequenceNumber:PASS
tests/lab-test.c:4834:test_makeAckPacket_nullPacket:PASS
tests/lab-test.c:4836:test_makeFinPacket_validPacket:PASS
tests/lab-test.c:4837:test_makeFinPacket_zeroSequenceNumber:PASS
tests/lab-test.c:4838:test_makeFinPacket_maxSequenceNumber:PASS
tests/lab-test.c:4839:test_makeFinPacket_nullPacket:PASS
tests/lab-test.c:4841:test_encodePacket_dataPacket:PASS
tests/lab-test.c:4842:test_encodePacket_ackPacket:PASS
tests/lab-test.c:4843:test_encodePacket_finPacket:PASS
tests/lab-test.c:4844:test_encodePacket_emptyDataPacket:PASS
tests/lab-test.c:4845:test_encodePacket_maxPayload:PASS
tests/lab-test.c:4846:test_encodePacket_nullPacket:PASS
tests/lab-test.c:4847:test_encodePacket_nullBuffer:PASS
tests/lab-test.c:4848:test_encodePacket_nullPacketSize:PASS
tests/lab-test.c:4849:test_encodePacket_bufferTooSmall:PASS
tests/lab-test.c:4850:test_encodePacket_invalidType:PASS
tests/lab-test.c:4851:test_encodePacket_ackWithPayload:PASS
tests/lab-test.c:4852:test_encodePacket_finWithPayload:PASS
tests/lab-test.c:4853:test_encodePacket_payloadTooLarge:PASS
tests/lab-test.c:4855:test_decodePacket_validDataPacket:PASS
tests/lab-test.c:4856:test_decodePacket_validAckPacket:PASS
tests/lab-test.c:4857:test_decodePacket_validFinPacket:PASS
tests/lab-test.c:4858:test_decodePacket_emptyDataPacket:PASS
tests/lab-test.c:4859:test_decodePacket_maxPayload:PASS
tests/lab-test.c:4860:test_decodePacket_nullBuffer:PASS
tests/lab-test.c:4861:test_decodePacket_nullPacket:PASS
tests/lab-test.c:4862:test_decodePacket_tooShort:PASS
tests/lab-test.c:4863:test_decodePacket_tooLarge:PASS
tests/lab-test.c:4864:test_decodePacket_invalidType:PASS
tests/lab-test.c:4865:test_decodePacket_nonzeroReservedByte:PASS
tests/lab-test.c:4866:test_decodePacket_lengthMismatch:PASS
tests/lab-test.c:4867:test_decodePacket_corruptedPayload:PASS
tests/lab-test.c:4868:test_decodePacket_corruptedSequenceNumber:PASS
tests/lab-test.c:4869:test_decodePacket_ackWithPayload:PASS
tests/lab-test.c:4870:test_decodePacket_finWithPayload:PASS
tests/lab-test.c:4872:test_senderInit_validArguments:PASS
tests/lab-test.c:4873:test_senderInit_minimumWindow:PASS
tests/lab-test.c:4874:test_senderInit_maximumWindow:PASS
tests/lab-test.c:4875:test_senderInit_nullPointer:PASS
tests/lab-test.c:4876:test_senderInit_zeroWindow:PASS
tests/lab-test.c:4877:test_senderInit_windowTooLarge:PASS
tests/lab-test.c:4878:test_senderInit_zeroTimeout:PASS
tests/lab-test.c:4879:test_senderInit_resetsState:PASS
tests/lab-test.c:4881:test_senderSendData_firstPacket:PASS
tests/lab-test.c:4882:test_senderSendData_storesPacket:PASS
tests/lab-test.c:4883:test_senderSendData_multiplePackets:PASS
tests/lab-test.c:4884:test_senderSendData_nullSender:PASS
tests/lab-test.c:4885:test_senderSendData_nullData:PASS
tests/lab-test.c:4886:test_senderSendData_nullPacket:PASS
tests/lab-test.c:4887:test_senderSendData_zeroLength:PASS
tests/lab-test.c:4888:test_senderSendData_payloadTooLarge:PASS
tests/lab-test.c:4889:test_senderSendData_fullWindow:PASS
tests/lab-test.c:4890:test_senderSendData_afterFinSent:PASS
tests/lab-test.c:4891:test_senderSendData_afterFinAcked:PASS
tests/lab-test.c:4893:test_senderHandleAck_advancesBase:PASS
tests/lab-test.c:4894:test_senderHandleAck_duplicateAck:PASS
tests/lab-test.c:4895:test_senderHandleAck_staleAck:PASS
tests/lab-test.c:4896:test_senderHandleAck_ackBeyondNext:PASS
tests/lab-test.c:4897:test_senderHandleAck_allDataAcknowledged:PASS
tests/lab-test.c:4898:test_senderHandleAck_noOutstandingData:PASS
tests/lab-test.c:4899:test_senderHandleAck_nullSender:PASS
tests/lab-test.c:4900:test_senderHandleAck_resetsTimeoutCount:PASS
tests/lab-test.c:4902:test_senderHandleTimeout_retransmitsAllOutstanding:PASS
tests/lab-test.c:4903:test_senderHandleTimeout_retransmitsUnacknowledgedOnly:PASS
tests/lab-test.c:4904:test_senderHandleTimeout_restartsTimer:PASS
tests/lab-test.c:4905:test_senderHandleTimeout_incrementsTimeoutCount:PASS
tests/lab-test.c:4906:test_senderHandleTimeout_timerNotExpired:PASS
tests/lab-test.c:4907:test_senderHandleTimeout_timerNotRunning:PASS
tests/lab-test.c:4908:test_senderHandleTimeout_nullSender:PASS
tests/lab-test.c:4909:test_senderHandleTimeout_nullPackets:PASS
tests/lab-test.c:4910:test_senderHandleTimeout_nullCount:PASS
tests/lab-test.c:4911:test_senderHandleTimeout_timeWentBackward:PASS
tests/lab-test.c:4912:test_senderHandleTimeout_exactExpiration:PASS
tests/lab-test.c:4913:test_senderHandleTimeout_noOutstandingPackets:PASS
tests/lab-test.c:4914:test_senderHandleTimeout_exceedsConfiguredWindow:PASS
tests/lab-test.c:4917:test_getTimeMs_returns_nonzero:PASS
tests/lab-test.c:4918:test_getTimeMs_is_monotonic:PASS
tests/lab-test.c:4919:test_getTimeMs_does_not_decrease_over_time:PASS
tests/lab-test.c:4920:test_getTimeMs_reasonable_timestamp:PASS
tests/lab-test.c:4922:test_senderTimerDue_null_sender:PASS
tests/lab-test.c:4923:test_senderTimerDue_null_remaining:PASS
tests/lab-test.c:4924:test_senderTimerDue_timer_not_running:PASS
tests/lab-test.c:4925:test_senderTimerDue_full_timeout_remaining:PASS
tests/lab-test.c:4926:test_senderTimerDue_partial_timeout_remaining:PASS
tests/lab-test.c:4927:test_senderTimerDue_timer_expired:PASS
tests/lab-test.c:4928:test_senderTimerDue_timer_expired_long_ago:PASS
tests/lab-test.c:4929:test_senderTimerDue_time_before_timer_start:PASS
tests/lab-test.c:4930:test_senderTimerDue_one_ms_before_expiration:PASS
tests/lab-test.c:4931:test_senderTimerDue_zero_timeout:PASS
tests/lab-test.c:4933:test_senderSendFin_null_sender:PASS
tests/lab-test.c:4934:test_senderSendFin_null_packet:PASS
tests/lab-test.c:4935:test_senderSendFin_unacknowledged_data:PASS
tests/lab-test.c:4936:test_senderSendFin_success:PASS
tests/lab-test.c:4937:test_senderSendFin_empty_file:PASS
tests/lab-test.c:4938:test_senderSendFin_already_sent:PASS
tests/lab-test.c:4939:test_senderSendFin_does_not_modify_state_on_failure:PASS
tests/lab-test.c:4941:test_senderHandleFinAck_null_sender:PASS
tests/lab-test.c:4942:test_senderHandleFinAck_fin_not_sent:PASS
tests/lab-test.c:4943:test_senderHandleFinAck_already_acknowledged:PASS
tests/lab-test.c:4944:test_senderHandleFinAck_wrong_ack_too_small:PASS
tests/lab-test.c:4945:test_senderHandleFinAck_wrong_ack_too_large:PASS
tests/lab-test.c:4946:test_senderHandleFinAck_success:PASS
tests/lab-test.c:4947:test_senderHandleFinAck_empty_file:PASS
tests/lab-test.c:4948:test_senderHandleFinAck_timer_already_stopped:PASS
tests/lab-test.c:4950:test_senderFinished_null_sender:PASS
tests/lab-test.c:4951:test_senderFinished_no_packets_sent:PASS
tests/lab-test.c:4952:test_senderFinished_data_not_fully_acknowledged:PASS
tests/lab-test.c:4953:test_senderFinished_fin_not_sent:PASS
tests/lab-test.c:4954:test_senderFinished_fin_not_acknowledged:PASS
tests/lab-test.c:4955:test_senderFinished_success:PASS
tests/lab-test.c:4956:test_senderFinished_empty_file_complete:PASS
tests/lab-test.c:4957:test_senderFinished_data_acknowledged_but_fin_not_sent:PASS
tests/lab-test.c:4959:test_receiverInit_null_receiver:PASS
tests/lab-test.c:4960:test_receiverInit_success:PASS
tests/lab-test.c:4961:test_receiverInit_resets_existing_state:PASS
tests/lab-test.c:4962:test_receiverInit_sets_activity_timestamp:PASS
tests/lab-test.c:4964:test_receiverHandleData_expeReceiver registration failed: receiver required
Receiver registration failed: no valid OK response after 5 attempts
Sender registration failed: receiver required
ctedPacket:PASS
tests/lab-test.c:4965:test_receiverHandleData_multipleInOrderPackets:PASS
tests/lab-test.c:4966:test_receiverHandleData_duplicatePacket:PASS
tests/lab-test.c:4967:test_receiverHandleData_outOfOrderPacket:PASS
tests/lab-test.c:4968:test_receiverHandleData_missingPacket:PASS
tests/lab-test.c:4969:test_receiverHandleData_zeroLengthPayload:PASS
tests/lab-test.c:4970:test_receiverHandleData_maxPayload:PASS
tests/lab-test.c:4971:test_receiverHandleData_nullReceiver:PASS
tests/lab-test.c:4972:test_receiverHandleData_nullPacket:PASS
tests/lab-test.c:4973:test_receiverHandleData_nullAck:PASS
tests/lab-test.c:4974:test_receiverHandleData_nonDataPacket:PASS
tests/lab-test.c:4975:test_receiverHandleData_oversizedPayload:PASS
tests/lab-test.c:4976:test_receiverHandleData_receiverFinished:PASS
tests/lab-test.c:4977:test_receiverHandleData_repeatedOutOfOrderPackets:PASS
tests/lab-test.c:4979:test_receiverHandleFin_nullReceiver:PASS
tests/lab-test.c:4980:test_receiverHandleFin_nullPacket:PASS
tests/lab-test.c:4981:test_receiverHandleFin_nullAck:PASS
tests/lab-test.c:4982:test_receiverHandleFin_nonFinPacket:PASS
tests/lab-test.c:4983:test_receiverHandleFin_nonzeroPayload:PASS
tests/lab-test.c:4984:test_receiverHandleFin_expectedSequence:PASS
tests/lab-test.c:4985:test_receiverHandleFin_emptyFile:PASS
tests/lab-test.c:4986:test_receiverHandleFin_outOfOrderSequence:PASS
tests/lab-test.c:4987:test_receiverHandleFin_earlyFin:PASS
tests/lab-test.c:4988:test_receiverHandleFin_duplicateFin:PASS
tests/lab-test.c:4989:test_receiverHandleFin_unexpectedFinAfterCompletion:PASS
tests/lab-test.c:4990:test_receiverHandleFin_activityTimestamp:PASS
tests/lab-test.c:4992:test_receiverTimerExpired_nullReceiver:PASS
tests/lab-test.c:4993:test_receiverTimerExpired_noElapsedTime:PASS
tests/lab-test.c:4994:test_receiverTimerExpired_lessThanTimeout:PASS
tests/lab-test.c:4995:test_receiverTimerExpired_exactlyAtTimeout:PASS
tests/lab-test.c:4996:test_receiverTimerExpired_pastTimeout:PASS
tests/lab-test.c:4997:test_receiverTimerExpired_timeBeforeLastActivity:PASS
tests/lab-test.c:4998:test_receiverTimerExpired_zeroLastActivity:PASS
tests/lab-test.c:4999:test_receiverTimerExpired_largeTimestamps:PASS
tests/lab-test.c:5000:test_receiverTimerExpired_doesNotModifyState:PASS
tests/lab-test.c:5002:test_resolveAddr_validIPv4:PASS
tests/lab-test.c:5003:test_resolveAddr_validHostname:PASS
tests/lab-test.c:5004:test_resolveAddr_invalidHostname:PASS
tests/lab-test.c:5005:test_resolveAddr_nullHost:PASS
tests/lab-test.c:5006:test_resolveAddr_nullPort:PASS
tests/lab-test.c:5007:test_resolveAddr_nullResults:PASS
tests/lab-test.c:5008:test_resolveAddr_failureClearsResults:PASS
tests/lab-test.c:5009:test_resolveAddr_numericService:PASS
tests/lab-test.c:5011:test_createSocket_validUDPIPv4:PASS
tests/lab-test.c:5012:test_createSocket_validUDPIPv6:PASS
tests/lab-test.c:5013:test_createSocket_nullAddress:PASS
tests/lab-test.c:5014:test_createSocket_invalidFamily:PASS
tests/lab-test.c:5015:test_createSocket_invalidSocketType:PASS
tests/lab-test.c:5016:test_createSocket_invalidProtocol:PASS
tests/lab-test.c:5018:test_registerReceiver_success:PASS
tests/lab-test.c:5019:test_registerReceiver_relayError:PASS
tests/lab-test.c:5020:test_registerReceiver_nullRelay:PASS
tests/lab-test.c:5021:test_registerReceiver_nullSession:PASS
tests/lab-test.c:5022:test_registerReceiver_invalidSessionCharacter:PASS
tests/lab-test.c:5023:test_registerReceiver_sessionTooLong:PASS
tests/lab-test.c:5024:test_registerReceiver_invalidSocket:PASS
tests/lab-test.c:5025:test_registerReceiver_ipv6_address_check:PASS
tests/lab-test.c:5026:test_registerReceiver_fails_after_five_attempts:PASS
tests/lab-test.c:5028:test_registerSender_success:PASS
tests/lab-test.c:5029:test_registerSender_relayError:PASS
tests/lab-test.c:5030:test_registerSender_nullRelay:PASS
tests/lab-test.c:5031:test_registerSender_nullSession:PASS
tests/lab-test.c:5032:test_registerSender_invalidSession:PASS
tests/lab-test.c:5033:test_registerSender_sessionTooLong:PASS
tests/lab-test.c:5034:test_registerSender_negativeLoss:PASS
tests/Sender registration failed after 5 attempts
runSender: invalid argument
runSender: invalid argument
runSender: invalid window or timeout
runSender: invalid window or timeout
runSender: invalid window or timeout
fopen: No such file or directory
runSender: failed to resolve relay
Sender registration failed: test rejection
runSender: relay registration failed
lab-test.c:5035:test_registerSender_corruptTooHigh:PASS
tests/lab-test.c:5036:test_registerSender_dupTooHigh:PASS
tests/lab-test.c:5037:test_registerSender_invalidSocket:PASS
tests/lab-test.c:5038:test_registerSender_ipv6_success:PASS
tests/lab-test.c:5039:test_registerSender_wrongRelayAddress:PASS
tests/lab-test.c:5041:test_runSender_nullRelay_returnsFailure:PASS
tests/lab-test.c:5042:test_runSender_nullFilename_returnsFailure:PASS
tests/lab-test.c:5043:test_runSender_zeroWindow_returnsFailure:PASS
tests/lab-test.c:5044:test_runSender_windowAboveMaximum_returnsFailure:PASS
tests/lab-test.c:5045:test_runSender_zeroTimeout_returnsFailure:PASS
tests/lab-test.c:5046:test_runSender_missingFile_returnsFailure:PASS
tests/lab-test.c:5047:test_runSender_invalidPort_returnsFailure:PASS
tests/lab-test.c:5048:test_runSender_relayRejectsRegistration_returnsFailure:PASS
tests/lab-test.c:5050:test_runReceiver_validDataAndFin:PASS
tests/lab-test.c:5051:test_runReceiver_emptyFile:PASS
tests/lab-test.c:5052:test_runReceiver_discardsCorruptedPacket:PASS
tests/lab-test.c:5053:test_runReceiver_outOfOrderData:PASS
tests/lab-test.c:5055:test_completeTransfer_withLossCorruptionAndDuplication:PASS

-----------------------
214 Tests 0 Failures 0 Ignored 
OK
mkdir -p ./build/report/html
mkdir -p ./build/report/txt
gcovr -r . --html --html-details --exclude-directories build/tests/harness --exclude '.*main\.c$' --exclude '.*test\.c$' -o ./build/report/html/coverage_report.html
(INFO) Reading coverage data...

(INFO) Writing coverage report...

gcovr -r . --txt                 --exclude-directories build/tests/harness --exclude '.*main\.c$' --exclude '.*test\.c$'
(INFO) Reading coverage data...

(INFO) Writing coverage report...

------------------------------------------------------------------------------
                           GCC Code Coverage Report
Directory: .
------------------------------------------------------------------------------
File                                       Lines     Exec  Cover   Missing
------------------------------------------------------------------------------
src/lab.c                                    718      430    59%   1176,1181,1183,1190-1191,1194,1196,1198-1201,1204-1205,1213,1215-1216,1218,1220,1224,1226,1228,1232,1235-1237,1245-1247,1249,1251,1253-1254,1256,1259,1261,1263,1266,1269-1271,1283-1284,1286-1288,1291,1293,1295,1297,1300,1302,1305-1306,1308,1310,1312,1315,1318-1320,1323,1325,1327,1329,1331,1334,1337-1339,1342-1343,1346,1354-1356,1358,1360-1362,1364,1368,1370-1372,1375-1376,1379,1381,1384-1385,1393,1395,1397,1401-1403,1406-1407,1414,1416-1420,1422,1425-1430,1432,1434,1437-1443,1447-1448,1456,1458,1461-1462,1465,1471-1472,1478,1481,1487,1493,1511,1514,1519-1522,1534-1535,1537-1539,1543-1546,1550-1553,1555-1557,1564-1567,1570-1571,1573-1574,1577-1578,1581-1583,1587-1589,1596,1604-1606,1608,1610-1612,1615-1616,1619-1620,1622-1624,1627,1630-1633,1636,1639,1641,1645-1647,1650-1651,1659-1660,1663,1670-1671,1673-1675,1678,1680,1682-1683,1686-1690,1693,1698,1700-1701,1704-1705,1707-1709,1716,1723,1725-1726,1729,1732-1734,1736,1739,1747,1753-1754,1756-1758,1760,1762-1764,1767-1768,1771-1772,1775-1776,1779,1781,1785-1787,1790-1791,1794-1795,1802-1803,1806,1808,1811,1813,1816,1819-1822,1827-1829,1832,1834-1836,1839-1840,1843-1846,1850,1853,1855,1868
------------------------------------------------------------------------------
TOTAL                                        718      430    59%
------------------------------------------------------------------------------
```

---

## Address Sanitizer Report

This section was generated by running `make leak-test` in the project root directory.

```bash
tests/lab-test.c:4816:test_packetChecksum_empty:PASS
tests/lab-test.c:4817:test_packetChecksum_singleByte:PASS
tests/lab-test.c:4818:test_packetChecksum_twoBytes:PASS
tests/lab-test.c:4819:test_packetChecksum_fourBytes:PASS
tests/lab-test.c:4820:test_packetChecksum_oddLength:PASS
tests/lab-test.c:4821:test_packetChecksum_oddLength:PASS
tests/lab-test.c:4823:test_makeDataPacket_validPacket:PASS
tests/lab-test.c:4824:test_makeDataPacket_emptyPayload:PASS
tests/lab-test.c:4825:test_makeDataPacket_maxPayload:PASS
tests/lab-test.c:4826:test_makeDataPacket_nullPacket:PASS
tests/lab-test.c:4827:test_makeDataPacket_nullDataWithNonzeroLength:PASS
tests/lab-test.c:4828:test_makeDataPacket_payloadTooLarge:PASS
tests/lab-test.c:4829:test_makeDataPacket_zeroLengthWithNonNullData:PASS
tests/lab-test.c:4831:test_makeAckPacket_validPacket:PASS
tests/lab-test.c:4832:test_makeAckPacket_zeroSequenceNumber:PASS
tests/lab-test.c:4833:test_makeAckPacket_maxSequenceNumber:PASS
tests/lab-test.c:4834:test_makeAckPacket_nullPacket:PASS
tests/lab-test.c:4836:test_makeFinPacket_validPacket:PASS
tests/lab-test.c:4837:test_makeFinPacket_zeroSequenceNumber:PASS
tests/lab-test.c:4838:test_makeFinPacket_maxSequenceNumber:PASS
tests/lab-test.c:4839:test_makeFinPacket_nullPacket:PASS
tests/lab-test.c:4841:test_encodePacket_dataPacket:PASS
tests/lab-test.c:4842:test_encodePacket_ackPacket:PASS
tests/lab-test.c:4843:test_encodePacket_finPacket:PASS
tests/lab-test.c:4844:test_encodePacket_emptyDataPacket:PASS
tests/lab-test.c:4845:test_encodePacket_maxPayload:PASS
tests/lab-test.c:4846:test_encodePacket_nullPacket:PASS
tests/lab-test.c:4847:test_encodePacket_nullBuffer:PASS
tests/lab-test.c:4848:test_encodePacket_nullPacketSize:PASS
tests/lab-test.c:4849:test_encodePacket_bufferTooSmall:PASS
tests/lab-test.c:4850:test_encodePacket_invalidType:PASS
tests/lab-test.c:4851:test_encodePacket_ackWithPayload:PASS
tests/lab-test.c:4852:test_encodePacket_finWithPayload:PASS
tests/lab-test.c:4853:test_encodePacket_payloadTooLarge:PASS
tests/lab-test.c:4855:test_decodePacket_validDataPacket:PASS
tests/lab-test.c:4856:test_decodePacket_validAckPacket:PASS
tests/lab-test.c:4857:test_decodePacket_validFinPacket:PASS
tests/lab-test.c:4858:test_decodePacket_emptyDataPacket:PASS
tests/lab-test.c:4859:test_decodePacket_maxPayload:PASS
tests/lab-test.c:4860:test_decodePacket_nullBuffer:PASS
tests/lab-test.c:4861:test_decodePacket_nullPacket:PASS
tests/lab-test.c:4862:test_decodePacket_tooShort:PASS
tests/lab-test.c:4863:test_decodePacket_tooLarge:PASS
tests/lab-test.c:4864:test_decodePacket_invalidType:PASS
tests/lab-test.c:4865:test_decodePacket_nonzeroReservedByte:PASS
tests/lab-test.c:4866:test_decodePacket_lengthMismatch:PASS
tests/lab-test.c:4867:test_decodePacket_corruptedPayload:PASS
tests/lab-test.c:4868:test_decodePacket_corruptedSequenceNumber:PASS
tests/lab-test.c:4869:test_decodePacket_ackWithPayload:PASS
tests/lab-test.c:4870:test_decodePacket_finWithPayload:PASS
tests/lab-test.c:4872:test_senderInit_validArguments:PASS
tests/lab-test.c:4873:test_senderInit_minimumWindow:PASS
tests/lab-test.c:4874:test_senderInit_maximumWindow:PASS
tests/lab-test.c:4875:test_senderInit_nullPointer:PASS
tests/lab-test.c:4876:test_senderInit_zeroWindow:PASS
tests/lab-test.c:4877:test_senderInit_windowTooLarge:PASS
tests/lab-test.c:4878:test_senderInit_zeroTimeout:PASS
tests/lab-test.c:4879:test_senderInit_resetsState:PASS
tests/lab-test.c:4881:test_senderSendData_firstPacket:PASS
tests/lab-test.c:4882:test_senderSendData_storesPacket:PASS
tests/lab-test.c:4883:test_senderSendData_multiplePackets:PASS
tests/lab-test.c:4884:test_senderSendData_nullSender:PASS
tests/lab-test.c:4885:test_senderSendData_nullData:PASS
tests/lab-test.c:4886:test_senderSendData_nullPacket:PASS
tests/lab-test.c:4887:test_senderSendData_zeroLength:PASS
tests/lab-test.c:4888:test_senderSendData_payloadTooLarge:PASS
tests/lab-test.c:4889:test_senderSendData_fullWindow:PASS
tests/lab-test.c:4890:test_senderSendData_afterFinSent:PASS
tests/lab-test.c:4891:test_senderSendData_afterFinAcked:PASS
tests/lab-test.c:4893:test_senderHandleAck_advancesBase:PASS
tests/lab-test.c:4894:test_senderHandleAck_duplicateAck:PASS
tests/lab-test.c:4895:test_senderHandleAck_staleAck:PASS
tests/lab-test.c:4896:test_senderHandleAck_ackBeyondNext:PASS
tests/lab-test.c:4897:test_senderHandleAck_allDataAcknowledged:PASS
tests/lab-test.c:4898:test_senderHandleAck_noOutstandingData:PASS
tests/lab-test.c:4899:test_senderHandleAck_nullSender:PASS
tests/lab-test.c:4900:test_senderHandleAck_resetsTimeoutCount:PASS
tests/lab-test.c:4902:test_senderHandleTimeout_retransmitsAllOutstanding:PASS
tests/lab-test.c:4903:test_senderHandleTimeout_retransmitsUnacknowledgedOnly:PASS
tests/lab-test.c:4904:test_senderHandleTimeout_restartsTimer:PASS
tests/lab-test.c:4905:test_senderHandleTimeout_incrementsTimeoutCount:PASS
tests/lab-test.c:4906:test_senderHandleTimeout_timerNotExpired:PASS
tests/lab-test.c:4907:test_senderHandleTimeout_timerNotRunning:PASS
tests/lab-test.c:4908:test_senderHandleTimeout_nullSender:PASS
tests/lab-test.c:4909:test_senderHandleTimeout_nullPackets:PASS
tests/lab-test.c:4910:test_senderHandleTimeout_nullCount:PASS
tests/lab-test.c:4911:test_senderHandleTimeout_timeWentBackward:PASS
tests/lab-test.c:4912:test_senderHandleTimeout_exactExpiration:PASS
tests/lab-test.c:4913:test_senderHandleTimeout_noOutstandingPackets:PASS
tests/lab-test.c:4914:test_senderHandleTimeout_exceedsConfiguredWindow:PASS
tests/lab-test.c:4917:test_getTimeMs_returns_nonzero:PASS
tests/lab-test.c:4918:test_getTimeMs_is_monotonic:PASS
tests/lab-test.c:4919:test_getTimeMs_does_not_decrease_over_time:PASS
tests/lab-test.c:4920:test_getTimeMs_reasonable_timestamp:PASS
tests/lab-test.c:4922:test_senderTimerDue_null_sender:PASS
tests/lab-test.c:4923:test_senderTimerDue_null_remaining:PASS
tests/lab-test.c:4924:test_senderTimerDue_timer_not_running:PASS
tests/lab-test.c:4925:test_senderTimerDue_full_timeout_remaining:PASS
tests/lab-test.c:4926:test_senderTimerDue_partial_timeout_remaining:PASS
tests/lab-test.c:4927:test_senderTimerDue_timer_expired:PASS
tests/lab-test.c:4928:test_senderTimerDue_timer_expired_long_ago:PASS
tests/lab-test.c:4929:test_senderTimerDue_time_before_timer_start:PASS
tests/lab-test.c:4930:test_senderTimerDue_one_ms_before_expiration:PASS
tests/lab-test.c:4931:test_senderTimerDue_zero_timeout:PASS
tests/lab-test.c:4933:test_senderSendFin_null_sender:PASS
tests/lab-test.c:4934:test_senderSendFin_null_packet:PASS
tests/lab-test.c:4935:test_senderSendFin_unacknowledged_data:PASS
tests/lab-test.c:4936:test_senderSendFin_success:PASS
tests/lab-test.c:4937:test_senderSendFin_empty_file:PASS
tests/lab-test.c:4938:test_senderSendFin_already_sent:PASS
tests/lab-test.c:4939:test_senderSendFin_does_not_modify_state_on_failure:PASS
tests/lab-test.c:4941:test_senderHandleFinAck_null_sender:PASS
tests/lab-test.c:4942:test_senderHandleFinAck_fin_not_sent:PASS
tests/lab-test.c:4943:test_senderHandleFinAck_already_acknowledged:PASS
tests/lab-test.c:4944:test_senderHandleFinAck_wrong_ack_too_small:PASS
tests/lab-test.c:4945:test_senderHandleFinAck_wrong_ack_too_large:PASS
tests/lab-test.c:4946:test_senderHandleFinAck_success:PASS
tests/lab-test.c:4947:test_senderHandleFinAck_empty_file:PASS
tests/lab-test.c:4948:test_senderHandleFinAck_timer_already_stopped:PASS
tests/lab-test.c:4950:test_senderFinished_null_sender:PASS
tests/lab-test.c:4951:test_senderFinished_no_packets_sent:PASS
tests/lab-test.c:4952:test_senderFinished_data_not_fully_acknowledged:PASS
tests/lab-test.c:4953:test_senderFinished_fin_not_sent:PASS
tests/lab-test.c:4954:test_senderFinished_fin_not_acknowledged:PASS
tests/lab-test.c:4955:test_senderFinished_success:PASS
tests/lab-test.c:4956:test_senderFinished_empty_file_complete:PASS
tests/lab-test.c:4957:test_senderFinished_data_acknowledged_but_fin_not_sent:PASS
tests/lab-test.c:4959:test_receiverInit_null_receiver:PASS
tests/lab-test.c:4960:test_receiverInit_success:PASS
tests/lab-test.c:4961:test_receiverInit_resets_existing_state:PASS
tests/lab-test.c:4962:test_receiverInit_sets_activity_timestamp:PASS
tests/lab-test.c:4964:test_receiverHandleData_expeReceiver registration failed: receiver required
Receiver registration failed: no valid OK response after 5 attempts
Sender registration failed: receiver required
ctedPacket:PASS
tests/lab-test.c:4965:test_receiverHandleData_multipleInOrderPackets:PASS
tests/lab-test.c:4966:test_receiverHandleData_duplicatePacket:PASS
tests/lab-test.c:4967:test_receiverHandleData_outOfOrderPacket:PASS
tests/lab-test.c:4968:test_receiverHandleData_missingPacket:PASS
tests/lab-test.c:4969:test_receiverHandleData_zeroLengthPayload:PASS
tests/lab-test.c:4970:test_receiverHandleData_maxPayload:PASS
tests/lab-test.c:4971:test_receiverHandleData_nullReceiver:PASS
tests/lab-test.c:4972:test_receiverHandleData_nullPacket:PASS
tests/lab-test.c:4973:test_receiverHandleData_nullAck:PASS
tests/lab-test.c:4974:test_receiverHandleData_nonDataPacket:PASS
tests/lab-test.c:4975:test_receiverHandleData_oversizedPayload:PASS
tests/lab-test.c:4976:test_receiverHandleData_receiverFinished:PASS
tests/lab-test.c:4977:test_receiverHandleData_repeatedOutOfOrderPackets:PASS
tests/lab-test.c:4979:test_receiverHandleFin_nullReceiver:PASS
tests/lab-test.c:4980:test_receiverHandleFin_nullPacket:PASS
tests/lab-test.c:4981:test_receiverHandleFin_nullAck:PASS
tests/lab-test.c:4982:test_receiverHandleFin_nonFinPacket:PASS
tests/lab-test.c:4983:test_receiverHandleFin_nonzeroPayload:PASS
tests/lab-test.c:4984:test_receiverHandleFin_expectedSequence:PASS
tests/lab-test.c:4985:test_receiverHandleFin_emptyFile:PASS
tests/lab-test.c:4986:test_receiverHandleFin_outOfOrderSequence:PASS
tests/lab-test.c:4987:test_receiverHandleFin_earlyFin:PASS
tests/lab-test.c:4988:test_receiverHandleFin_duplicateFin:PASS
tests/lab-test.c:4989:test_receiverHandleFin_unexpectedFinAfterCompletion:PASS
tests/lab-test.c:4990:test_receiverHandleFin_activityTimestamp:PASS
tests/lab-test.c:4992:test_receiverTimerExpired_nullReceiver:PASS
tests/lab-test.c:4993:test_receiverTimerExpired_noElapsedTime:PASS
tests/lab-test.c:4994:test_receiverTimerExpired_lessThanTimeout:PASS
tests/lab-test.c:4995:test_receiverTimerExpired_exactlyAtTimeout:PASS
tests/lab-test.c:4996:test_receiverTimerExpired_pastTimeout:PASS
tests/lab-test.c:4997:test_receiverTimerExpired_timeBeforeLastActivity:PASS
tests/lab-test.c:4998:test_receiverTimerExpired_zeroLastActivity:PASS
tests/lab-test.c:4999:test_receiverTimerExpired_largeTimestamps:PASS
tests/lab-test.c:5000:test_receiverTimerExpired_doesNotModifyState:PASS
tests/lab-test.c:5002:test_resolveAddr_validIPv4:PASS
tests/lab-test.c:5003:test_resolveAddr_validHostname:PASS
tests/lab-test.c:5004:test_resolveAddr_invalidHostname:PASS
tests/lab-test.c:5005:test_resolveAddr_nullHost:PASS
tests/lab-test.c:5006:test_resolveAddr_nullPort:PASS
tests/lab-test.c:5007:test_resolveAddr_nullResults:PASS
tests/lab-test.c:5008:test_resolveAddr_failureClearsResults:PASS
tests/lab-test.c:5009:test_resolveAddr_numericService:PASS
tests/lab-test.c:5011:test_createSocket_validUDPIPv4:PASS
tests/lab-test.c:5012:test_createSocket_validUDPIPv6:PASS
tests/lab-test.c:5013:test_createSocket_nullAddress:PASS
tests/lab-test.c:5014:test_createSocket_invalidFamily:PASS
tests/lab-test.c:5015:test_createSocket_invalidSocketType:PASS
tests/lab-test.c:5016:test_createSocket_invalidProtocol:PASS
tests/lab-test.c:5018:test_registerReceiver_success:PASS
tests/lab-test.c:5019:test_registerReceiver_relayError:PASS
tests/lab-test.c:5020:test_registerReceiver_nullRelay:PASS
tests/lab-test.c:5021:test_registerReceiver_nullSession:PASS
tests/lab-test.c:5022:test_registerReceiver_invalidSessionCharacter:PASS
tests/lab-test.c:5023:test_registerReceiver_sessionTooLong:PASS
tests/lab-test.c:5024:test_registerReceiver_invalidSocket:PASS
tests/lab-test.c:5025:test_registerReceiver_ipv6_address_check:PASS
tests/lab-test.c:5026:test_registerReceiver_fails_after_five_attempts:PASS
tests/lab-test.c:5028:test_registerSender_success:PASS
tests/lab-test.c:5029:test_registerSender_relayError:PASS
tests/lab-test.c:5030:test_registerSender_nullRelay:PASS
tests/lab-test.c:5031:test_registerSender_nullSession:PASS
tests/lab-test.c:5032:test_registerSender_invalidSession:PASS
tests/lab-test.c:5033:test_registerSender_sessionTooLong:PASS
tests/lab-test.c:5034:test_registerSender_negativeLoss:PASS
tests/Sender registration failed after 5 attempts
runSender: invalid argument
runSender: invalid argument
runSender: invalid window or timeout
runSender: invalid window or timeout
runSender: invalid window or timeout
fopen: No such file or directory
runSender: failed to resolve relay
Sender registration failed: test rejection
runSender: relay registration failed
lab-test.c:5035:test_registerSender_corruptTooHigh:PASS
tests/lab-test.c:5036:test_registerSender_dupTooHigh:PASS
tests/lab-test.c:5037:test_registerSender_invalidSocket:PASS
tests/lab-test.c:5038:test_registerSender_ipv6_success:PASS
tests/lab-test.c:5039:test_registerSender_wrongRelayAddress:PASS
tests/lab-test.c:5041:test_runSender_nullRelay_returnsFailure:PASS
tests/lab-test.c:5042:test_runSender_nullFilename_returnsFailure:PASS
tests/lab-test.c:5043:test_runSender_zeroWindow_returnsFailure:PASS
tests/lab-test.c:5044:test_runSender_windowAboveMaximum_returnsFailure:PASS
tests/lab-test.c:5045:test_runSender_zeroTimeout_returnsFailure:PASS
tests/lab-test.c:5046:test_runSender_missingFile_returnsFailure:PASS
tests/lab-test.c:5047:test_runSender_invalidPort_returnsFailure:PASS
tests/lab-test.c:5048:test_runSender_relayRejectsRegistration_returnsFailure:PASS
tests/lab-test.c:5050:test_runReceiver_validDataAndFin:PASS
tests/lab-test.c:5051:test_runReceiver_emptyFile:PASS
tests/lab-test.c:5052:test_runReceiver_discardsCorruptedPacket:PASS
tests/lab-test.c:5053:test_runReceiver_outOfOrderData:PASS
tests/lab-test.c:5055:test_completeTransfer_withLossCorruptionAndDuplication:PASS

-----------------------
214 Tests 0 Failures 0 Ignored 
OK
```

---

## Src Files
### lab.c

```c

#include "lab.h"

#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>


uint16_t packetChecksum(const uint8_t *data, size_t length)
{
    uint32_t sum = 0U;

    while (length > 1U) {
        /* Combine two bytes into one 16-bit word. */
        sum += ((uint32_t)data[0] << 8U) | (uint32_t)data[1];

        data += 2;
        length -= 2U;
    }

    if (length > 0U) {
        sum += (uint32_t)data[0] << 8U;
    }

    /* Fold carries back into the lower 16 bits. */
    while ((sum >> 16U) != 0U) {
        sum = (sum & UINT32_C(0xffff)) + (sum >> 16U);
    }

    return (uint16_t)(~sum & UINT32_C(0xffff));
}


int encodePacket(const Packet *packet, uint8_t *buffer, size_t buffer_size, size_t *packet_size)
{
    uint16_t network_length;
    uint32_t network_seq;
    size_t total_size;

    /* Validate pointers. */
    if (packet == NULL || buffer == NULL || packet_size == NULL) {
        return 1;
    }

    /* Validate packet type and payload length. */
    if (packet->type != PACKET_DATA &&
        packet->type != PACKET_ACK &&
        packet->type != PACKET_FIN) {
        return 1;
    }

    if (packet->length > MAX_PAYLOAD) {
        return 1;
    }

    /* ACK and FIN packets must not contain payloads. */
    if ((packet->type == PACKET_ACK || packet->type == PACKET_FIN) &&
        packet->length != 0) {
        return 1;
    }

    /* Calculate the encoded packet size. */
    total_size = (size_t)HEADER_SIZE + (size_t)packet->length;

    if (buffer_size < total_size) {
        return 1;
    }

    /* Initialize the header. */
    buffer[0] = packet->type;
    buffer[1] = 0; /* Reserved byte. */
    buffer[2] = 0; /* Checksum high byte. */
    buffer[3] = 0; /* Checksum low byte. */

    /* Encode the sequence number in network byte order. */
    network_seq = htonl(packet->seq);
    memcpy(&buffer[4], &network_seq, sizeof(network_seq));

    /* Encode the payload length in network byte order. */
    network_length = htons(packet->length);
    memcpy(&buffer[8], &network_length, sizeof(network_length));

    /* Copy the payload, if present. */
    if (packet->length > 0) {
        memcpy(&buffer[HEADER_SIZE], packet->payload, packet->length);
    }

    /* Calculate the checksum with bytes 2 and 3 set to zero. */
    uint16_t checksum = packetChecksum(buffer, total_size);

    /* Store the checksum in network byte order. */
    uint16_t network_checksum = htons(checksum);
    memcpy(&buffer[2], &network_checksum, sizeof(network_checksum));

    /* Report the encoded packet size. */
    *packet_size = total_size;

    return 0;
}

int decodePacket(const uint8_t *buffer, size_t buffer_size, Packet *packet)
{
    uint16_t network_checksum;
    uint16_t received_checksum;
    uint16_t network_length;
    uint16_t length;
    uint32_t network_seq;
    uint32_t seq;
    uint8_t type;
    uint8_t reserved;

    /* Validate pointers and minimum header size. */
    if (buffer == NULL || packet == NULL) {
        return 1;
    }

    if (buffer_size < HEADER_SIZE || buffer_size > MAX_PACKET_SIZE) {
        return 1;
    }

    /* Read the single-byte header fields. */
    type = buffer[0];
    reserved = buffer[1];

    /* Validate packet type and reserved byte. */
    if (type != PACKET_DATA &&
        type != PACKET_ACK &&
        type != PACKET_FIN) {
        return 1;
    }

    if (reserved != 0) {
        return 1;
    }

    /* Read the checksum and convert from network byte order. */
    memcpy(&network_checksum, &buffer[2], sizeof(network_checksum));
    received_checksum = ntohs(network_checksum);

    /* Read and convert the sequence number. */
    memcpy(&network_seq, &buffer[4], sizeof(network_seq));
    seq = ntohl(network_seq);

    /* Read and convert the payload length. */
    memcpy(&network_length, &buffer[8], sizeof(network_length));
    length = ntohs(network_length);

    /* Reject payloads that exceed the protocol maximum. */
    if (length > MAX_PAYLOAD) {
        return 1;
    }

    /* Ensure the datagram size matches its declared payload length. */
    if (buffer_size != HEADER_SIZE + (size_t)length) {
        return 1;
    }

    /* ACK and FIN packets must have empty payloads. */
    if ((type == PACKET_ACK || type == PACKET_FIN) && length != 0) {
        return 1;
    }

    /*
     * Verify the checksum without modifying the received buffer.
     * The checksum field must be zero during calculation.
     */
    uint8_t checksum_buffer[MAX_PACKET_SIZE];

    memcpy(checksum_buffer, buffer, buffer_size);
    checksum_buffer[2] = 0;
    checksum_buffer[3] = 0;

    uint16_t calculated_checksum =
        packetChecksum(checksum_buffer, buffer_size);

    if (calculated_checksum != received_checksum) {
        return 1;
    }

    /*
     * All validation has succeeded.
     * Only now copy the decoded fields into the output packet.
     */
    packet->type = type;
    packet->seq = seq;
    packet->length = length;

    if (length > 0) {
        memcpy(packet->payload, &buffer[HEADER_SIZE], length);
    }

    return 0;
}

int makeDataPacket(Packet *packet, uint32_t seq, const uint8_t *data, uint16_t length)
{
    /* Validate the packet pointer. */
    if (packet == NULL) {
        return 1;
    }

    /* Validate the payload length. */
    if (length > MAX_PAYLOAD) {
        return 1;
    }

    /* A nonempty payload requires a valid data pointer. */
    if (length > 0 && data == NULL) {
        return 1;
    }

    /* Initialize the packet header fields. */
    packet->type = PACKET_DATA;
    packet->seq = seq;
    packet->length = length;

    /* Copy the payload when there is data to copy. */
    if (length > 0) {
        memcpy(packet->payload, data, length);
    }

    return 0;
}

int makeAckPacket(Packet *packet, uint32_t seq)
{
    /* Validate the packet pointer. */
    if (packet == NULL) {
        return 1;
    }

    /* Initialize the ACK packet header fields. */
    packet->type = PACKET_ACK;
    packet->seq = seq;
    packet->length = 0;

    return 0;
}

int makeFinPacket(Packet *packet, uint32_t seq)
{
    /* Validate the packet pointer. */
    if (packet == NULL) {
        return 1;
    }

    /* Initialize the FIN packet header fields. */
    packet->type = PACKET_FIN;
    packet->seq = seq;
    packet->length = 0;

    return 0;
}

int senderInit(SenderState *sender, uint32_t window, uint32_t timeout_ms)
{
    /* Validate the sender pointer. */
    if (sender == NULL) {
        return 1;
    }

    /* Validate the window size. */
    if (window == 0 || window > MAX_WINDOW) {
        return 1;
    }

    /* Validate the retransmission timeout. */
    if (timeout_ms == 0) {
        return 1;
    }

    /* Initialize the sender state. */
    sender->base = 0;
    sender->nextseqnum = 0;
    sender->window = window;
    sender->timeout_ms = timeout_ms;
    sender->consecutive_timeouts = 0;
    sender->timer_start = 0;
    sender->timer_running = 0;

    /* Clear stored packets used for retransmission. */
    memset(sender->packets, 0, sizeof(sender->packets));

    /* Initialize FIN state. */
    memset(&sender->fin, 0, sizeof(sender->fin));
    sender->fin_sent = 0;
    sender->fin_acked = 0;

    return 0;
}

int senderSendData(SenderState *sender, const uint8_t *data, uint16_t length, uint64_t now_ms, Packet *packet)
{
    uint32_t window_index;

    /* Validate arguments. */
    if (sender == NULL || packet == NULL ||
        data == NULL || length == 0 || length > MAX_PAYLOAD) {
        return 1;
    }

    /* Reject sending if the window is full. */
    if (sender->nextseqnum - sender->base >= sender->window) {
        return 1;
    }

    /* Do not send data after the FIN packet has been sent. */
    if (sender->fin_sent || sender->fin_acked) {
        return 1;
    }

    /*
     * Use a circular buffer to store packets in the retransmission
     * window. This allows the sender to support windows up to MAX_WINDOW.
     */
    window_index = sender->nextseqnum % MAX_WINDOW;

    // GCOVR_EXCL_START // something crazy would have to happen for this to fail since anything that makes makeDataPacket return   1 wouldn't have gotten this far
    /* Create the next DATA packet. */
    if (makeDataPacket(packet, sender->nextseqnum, data, length) != 0) {
        return 1;
    }
    // GCOVR_EXCL_STOP

    /* Keep a copy so the packet can be retransmitted after a timeout. */
    sender->packets[window_index] = *packet;

    /* Advance the next sequence number. */
    sender->nextseqnum++;

    /*
     * Start the timer only when the first unacknowledged packet
     * enters an otherwise empty window.
     */
    if (sender->base == sender->nextseqnum - 1) {
        sender->timer_start = now_ms;
        sender->timer_running = 1;
    }

    return 0;
}

int senderHandleAck(SenderState *sender, uint32_t ack_seq, uint64_t now_ms)
{
    if (sender == NULL) {
        return 1;
    }

    /*
     * An ACK cannot acknowledge a sequence number beyond the
     * next sequence number that has been sent.
     */
    if (ack_seq > sender->nextseqnum) {
        return 1;
    }

    /*
     * Duplicate or stale ACKs do not advance the sender's base.
     */
    if (ack_seq <= sender->base) {
        return 0;
    }

    /*
     * Advance base to the next packet the receiver expects.
     * Because ACKs are cumulative, all packets before ack_seq
     * are now acknowledged.
     */
    sender->base = ack_seq;

    /*
     * If no DATA packets remain unacknowledged, stop the timer.
     * Otherwise, restart the timer for the oldest outstanding
     * packet.
     */
    if (sender->base == sender->nextseqnum) {
        sender->timer_running = 0;
        sender->timer_start = 0;
    } else {
        sender->timer_start = now_ms;
        sender->timer_running = 1;
    }

    /* A valid advancing ACK resets the consecutive timeout count. */
    sender->consecutive_timeouts = 0;

    return 0;
}

int senderHandleTimeout(SenderState *sender, uint64_t now_ms, Packet *packets, size_t *count)
{
    uint32_t seq;
    size_t num_packets = 0;

    /* Validate arguments. */
    if (sender == NULL || packets == NULL || count == NULL) {
        return 1;
    }

    *count = 0;

    /* A timeout can only be handled when the timer is running. */
    if (!sender->timer_running) {
        return 1;
    }

    /* Check whether the retransmission timer has expired. */
    if (now_ms < sender->timer_start ||
        now_ms - sender->timer_start < sender->timeout_ms) {
        return 1;
    }

    /* No outstanding packets means there is nothing to retransmit. */
    if (sender->base == sender->nextseqnum) {
        sender->timer_running = 0;
        sender->timer_start = 0;
        return 0;
    }

    /* Ensure the outstanding packet count is within the configured window. */
    if (sender->nextseqnum - sender->base > sender->window ||
        sender->nextseqnum - sender->base > MAX_WINDOW) {
        return 1;
    }

    /*
     * Go-Back-N: retransmit every outstanding DATA packet,
     * starting with the oldest unacknowledged packet.
     */
    for (seq = sender->base; seq < sender->nextseqnum; seq++) {
        uint32_t index = seq % MAX_WINDOW;

        packets[num_packets] = sender->packets[index];
        num_packets++;
    }

    *count = num_packets;

    /* Restart the timer for the retransmitted window. */
    sender->timer_start = now_ms;
    sender->timer_running = 1;

    /* Record the timeout for retry-limit handling. */
    sender->consecutive_timeouts++;

    return 0;
}

int senderTimerDue(const SenderState *sender, uint64_t now_ms, uint64_t *remaining_ms)
{
    uint64_t elapsed;

    /* Validate arguments. */
    if (sender == NULL || remaining_ms == NULL) {
        return 1;
    }

    *remaining_ms = 0;

    /* No active timer means there is no deadline to report. */
    if (!sender->timer_running) {
        return 1;
    }

    /* Guard against a timestamp earlier than the timer start. */
    if (now_ms < sender->timer_start) {
        *remaining_ms = sender->timeout_ms;
        return 0;
    }

    elapsed = now_ms - sender->timer_start;

    /* The timer has expired. */
    if (elapsed >= sender->timeout_ms) {
        *remaining_ms = 0;
        return 0;
    }

    /* Calculate the time remaining until expiration. */
    *remaining_ms = (uint64_t)sender->timeout_ms - elapsed;

    return 0;
}

int senderSendFin(SenderState *sender, uint64_t now_ms, Packet *packet)
{
    /* Validate arguments. */
    if (sender == NULL || packet == NULL) {
        return 1;
    }

    /* Do not send FIN while DATA packets remain unacknowledged. */
    if (sender->base != sender->nextseqnum) {
        return 1;
    }

    /* Do not send FIN more than once. */
    if (sender->fin_sent) {
        return 1;
    }

    // GCOVR_EXCL_START
    /* Create the FIN packet using the next sequence number. */
    if (makeFinPacket(packet, sender->nextseqnum) != 0) {
        return 1;
    }
    // GCOVR_EXCL_STOP

    /* Store a copy for retransmission if the FIN is lost. */
    sender->fin = *packet;

    /* Record that FIN has been sent. */
    sender->fin_sent = 1;
    sender->fin_acked = 0;

    /* Start the retransmission timer. */
    sender->timer_start = now_ms;
    sender->timer_running = 1;

    /* Reset the consecutive timeout counter for the FIN phase. */
    sender->consecutive_timeouts = 0;

    return 0;
}

int senderHandleFinAck(SenderState *sender, uint32_t ack_seq, uint64_t now_ms)
{
    (void)now_ms; /* Not needed for this implementation. */

    /* Validate the sender pointer. */
    if (sender == NULL) {
        return 1;
    }

    /* FIN must have been sent before its ACK can be accepted. */
    if (!sender->fin_sent) {
        return 1;
    }

    /* Ignore an ACK if FIN has already been acknowledged. */
    if (sender->fin_acked) {
        return 1;
    }

    /* The ACK must acknowledge the FIN sequence number. */
    if (ack_seq != sender->fin.seq + 1U) {
        return 1;
    }

    /* Mark FIN as acknowledged. */
    sender->fin_acked = 1;

    /* Stop the retransmission timer. */
    sender->timer_running = 0;
    sender->timer_start = 0;

    return 0;
}

int senderFinished(const SenderState *sender)
{
    /* Validate the sender pointer. */
    if (sender == NULL) {
        return 0;
    }

    /* All DATA packets and the FIN packet must be acknowledged. */
    if (sender->base == sender->nextseqnum &&
        sender->fin_sent &&
        sender->fin_acked) {
        return 1;
    }

    return 0;
}

int receiverInit(ReceiverState *receiver)
{
    /* Validate the receiver pointer. */
    if (receiver == NULL) {
        return 1;
    }

    /* Initialize the receiver state. */
    receiver->expectedseqnum = 0;
    receiver->finished = 0;
    receiver->last_activity = getTimeMs(); 

    return 0;
}

int receiverHandleData(ReceiverState *receiver, const Packet *packet, uint64_t now_ms, Packet *ack)
{
    if (receiver == NULL || packet == NULL || ack == NULL) {
        return 1;
    }

    /* Only DATA packets can be handled by this function. */
    if (packet->type != PACKET_DATA) {
        return 1;
    }

    /* Reject malformed packets with an oversized payload. */
    if (packet->length > MAX_PAYLOAD) {
        return 1;
    }

    /* Do not process data after the receiver has finished. */
    if (receiver->finished) {
        return 1;
    }

    /*
     * Go-Back-N receiver behavior:
     * accept only the next expected sequence number.
     */
    if (packet->seq == receiver->expectedseqnum) {
        receiver->expectedseqnum++;
        receiver->last_activity = now_ms;
    }

    /*
     * For an expected packet, duplicate, or out-of-order packet,
     * acknowledge the next expected sequence number. This is a
     * cumulative ACK: it tells the sender which sequence number
     * the receiver is waiting for next.
     */
    // GCOVR_EXCL_START
    if (makeAckPacket(ack, receiver->expectedseqnum) != 0) {
        return 1;
    }
    // GCOVR_EXCL_STOP

    return 0;
}

int receiverHandleFin(ReceiverState *receiver, const Packet *packet, uint64_t now_ms, Packet *ack)
{
    if (receiver == NULL || packet == NULL || ack == NULL) {
        return 1;
    }

    /* Only FIN packets can be handled by this function. */
    if (packet->type != PACKET_FIN) {
        return 1;
    }

    /* FIN packets must not contain a payload. */
    if (packet->length != 0) {
        return 1;
    }

    /*
     * If the receiver has already finished, acknowledge a repeated
     * FIN with the final cumulative ACK during the linger period.
     */
    if (receiver->finished) {
        // GCOVR_EXCL_START
        if (packet->seq == receiver->expectedseqnum - 1) {
            if (makeAckPacket(ack, receiver->expectedseqnum) != 0) {
                return 1;
            }
        // GCOVR_EXCL_STOP

            receiver->last_activity = now_ms;
            return 0;
        }

        return 1;
    }

    /*
     * Accept FIN only when its sequence number equals the next
     * expected DATA sequence number.
     */
    if (packet->seq != receiver->expectedseqnum) {
        /* FIN arrived too early or out of order; ACK current expectation. */
        // GCOVR_EXCL_START
        if (makeAckPacket(ack, receiver->expectedseqnum) != 0) {
            return 1;
        }
        // GCOVR_EXCL_STOP

        return 0;
    }

    /*
     * The FIN is in order. Advance the expected sequence number,
     * mark the transfer as finished, and generate the final ACK.
     */
    receiver->expectedseqnum++;
    receiver->finished = 1;
    receiver->last_activity = now_ms;

    // GCOVR_EXCL_START
    if (makeAckPacket(ack, receiver->expectedseqnum) != 0) {
        return 1;
    }
    // GCOVR_EXCL_STOP

    return 0;
}

int receiverTimerExpired(const ReceiverState *receiver, uint64_t now_ms)
{
    /* Reject an invalid receiver pointer. */
    if (receiver == NULL) {
        return 1;
    }

    /* Avoid unsigned underflow if the supplied time is earlier. */
    if (now_ms < receiver->last_activity) {
        return 0;
    }

    /* Expire after 30 seconds without activity. */
    if (now_ms - receiver->last_activity >= 30000) {
        return 1;
    }

    return 0;
}

uint64_t getTimeMs(void)
{
    struct timespec ts;

    // GCOVR_EXCL_START
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    // GCOVR_EXCL_STOP

    return ((uint64_t)ts.tv_sec * UINT64_C(1000))
           + ((uint64_t)ts.tv_nsec / UINT64_C(1000000));
}

int resolveAddr(const char *host, const char *port, struct addrinfo **results)
{
    struct addrinfo hints;
    int status;

    /* Validate arguments. */
    if (host == NULL || port == NULL || results == NULL) {
        return 1;
    }

    /* Initialize the output pointer before attempting resolution. */
    *results = NULL;

    /* Configure address resolution for UDP. */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;       /* IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM;    /* UDP */
    hints.ai_protocol = IPPROTO_UDP;

    /* Resolve the host and service (port). */
    status = getaddrinfo(host, port, &hints, results);

    if (status != 0) {
        *results = NULL;
        return 1;
    }

    return 0;
}

int createSocket(const struct addrinfo *addr)
{
    int sockfd;

    if (addr == NULL) {
        return -1;
    }

    sockfd = socket(addr->ai_family,
                    addr->ai_socktype,
                    addr->ai_protocol);

    if (sockfd == -1) {
        return -1;
    }

    return sockfd;
}

int registerReceiver(int sockfd, const struct sockaddr *relay, socklen_t relay_len, const char *session)
{
    char hello[64];
    char response[256];

    /* Validate arguments. */
    if (sockfd < 0 || relay == NULL || session == NULL) {
        return 1;
    }

    /* Validate the session length: 1 to 32 characters. */
    size_t session_len = strlen(session);

    if (session_len < 1 || session_len > 32) {
        return 1;
    }

    /* Validate session characters: a-z, 0-9, and hyphen. */
    for (size_t i = 0; i < session_len; i++) {
        char c = session[i];

        if (!((c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '-')) {
            return 1;
        }
    }

    /* Construct the registration message without a newline. */
    int hello_len = snprintf(hello, sizeof(hello),
                             "HELLO %s recv", session);
    // GCOVR_EXCL_START
    if (hello_len < 0 || (size_t)hello_len >= sizeof(hello)) {
        return 1;
    }
    // GCOVR_EXCL_STOP

    /* Set a one-second receive timeout. */
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    // GCOVR_EXCL_START
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout)) < 0) {
        return 1;
    }
    // GCOVR_EXCL_STOP

    /* Retry registration up to five times. */
    for (int attempt = 0; attempt < 5; attempt++) {

        ssize_t sent = sendto(sockfd,
                              hello,
                              (size_t)hello_len,
                              0,
                              relay,
                              relay_len);
        // GCOVR_EXCL_START    
        if (sent < 0 || sent != hello_len) {
            continue;
        }
        // GCOVR_EXCL_STOP

        struct sockaddr_storage sender_addr;
        socklen_t sender_len = sizeof(sender_addr);

        ssize_t received = recvfrom(sockfd,
                                    response,
                                    sizeof(response) - 1,
                                    0,
                                    (struct sockaddr *)&sender_addr,
                                    &sender_len);

        if (received < 0) {
            // GCOVR_EXCL_START
            if (errno == EAGAIN || errno == EWOULDBLOCK ||
                errno == EINTR) {
                continue;
            }
            // GCOVR_EXCL_STOP

            return 1;
        }

        response[received] = '\0';

        // /* Ignore datagrams that did not come from the relay. */
        // if (sender_len != relay_len ||
        //     sender_addr.ss_family != relay->sa_family) {
        //     continue;
        // } FIXME

        int same_address = 0;

        if (relay->sa_family == AF_INET) {
            const struct sockaddr_in *expected =
                (const struct sockaddr_in *)relay;
            const struct sockaddr_in *actual =
                (const struct sockaddr_in *)&sender_addr;

            same_address =
                expected->sin_port == actual->sin_port &&
                expected->sin_addr.s_addr == actual->sin_addr.s_addr;
        } else if (relay->sa_family == AF_INET6) {
            const struct sockaddr_in6 *expected =
                (const struct sockaddr_in6 *)relay;
            const struct sockaddr_in6 *actual =
                (const struct sockaddr_in6 *)&sender_addr;

            same_address =
                expected->sin6_port == actual->sin6_port &&
                memcmp(&expected->sin6_addr, &actual->sin6_addr,
                       sizeof(expected->sin6_addr)) == 0 &&
                expected->sin6_scope_id == actual->sin6_scope_id;
        }

        if (!same_address) {
            continue;
        }

        /* Registration succeeded. */
        if (strcmp(response, "OK") == 0) {
            return 0;
        }

        /* The relay explicitly rejected registration. */
        if (strncmp(response, "ERR ", 4) == 0) {
            fprintf(stderr, "Receiver registration failed: %s\n",
                    response + 4);
            return 1;
        }

        /* Unexpected response: retry registration. */
    }

    fprintf(stderr,
            "Receiver registration failed: no valid OK response "
            "after 5 attempts\n");

    return 1;
}

int registerSender(int sockfd, const struct sockaddr *relay, socklen_t relay_len, const char *session, double loss, double corrupt, double dup)
{
    char hello[128];
    char response[256];
    size_t session_len;
    int hello_len;

    /* Validate arguments. */
    if (sockfd < 0 || relay == NULL || session == NULL) {
        return 1;
    }

    /* Validate session length. */
    session_len = strlen(session);

    if (session_len < 1 || session_len > 32) {
        return 1;
    }

    /* Validate session characters. */
    for (size_t i = 0; i < session_len; i++) {
        char c = session[i];

        if (!((c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '-')) {
            return 1;
        }
    }

    /* Validate probabilities: each must be between 0 and 0.5. */
    if (loss < 0.0 || loss > 0.5 ||
        corrupt < 0.0 || corrupt > 0.5 ||
        dup < 0.0 || dup > 0.5) {
        return 1;
    }

    /* Construct the HELLO message without a newline. */
    hello_len = snprintf(hello, sizeof(hello),
                         "HELLO %s send %.6g %.6g %.6g",
                         session, loss, corrupt, dup);

    // GCOVR_EXCL_START
    if (hello_len < 0 || (size_t)hello_len >= sizeof(hello)) {
        return 1;
    }
    // GCOVR_EXCL_STOP

    /* Set a one-second receive timeout. */
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    // GCOVR_EXCL_START
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        return 1;
    }
    // GCOVR_EXCL_STOP

    /* Attempt registration up to five times. */
    for (int attempt = 0; attempt < 5; attempt++) {
        ssize_t sent = sendto(sockfd,
                              hello,
                              (size_t)hello_len,
                              0,
                              relay,
                              relay_len);

        // GCOVR_EXCL_START                      
        if (sent < 0 || sent != hello_len) {
            continue;
        }
        // GCOVR_EXCL_STOP

        struct sockaddr_storage sender_addr;
        socklen_t sender_len = sizeof(sender_addr);

        ssize_t received = recvfrom(sockfd,
                                    response,
                                    sizeof(response) - 1,
                                    0,
                                    (struct sockaddr *)&sender_addr,
                                    &sender_len);
        // GCOVR_EXCL_START    
        if (received < 0) {
            if (errno == EINTR ||
                errno == EAGAIN ||
                errno == EWOULDBLOCK) {
                continue;
            }

            perror("recvfrom");
            return 1;
        }
        // GCOVR_EXCL_STOP

        response[received] = '\0';

        // /* Verify that the response came from the relay. */
        // if (sender_len != relay_len ||
        //     sender_addr.ss_family != relay->sa_family) {
        //     continue;
        // } FIXME

        int same_address = 0;

        if (relay->sa_family == AF_INET) {
            const struct sockaddr_in *expected =
                (const struct sockaddr_in *)relay;
            const struct sockaddr_in *actual =
                (const struct sockaddr_in *)&sender_addr;

            same_address =
                expected->sin_port == actual->sin_port &&
                expected->sin_addr.s_addr == actual->sin_addr.s_addr;
        } else if (relay->sa_family == AF_INET6) {
            const struct sockaddr_in6 *expected =
                (const struct sockaddr_in6 *)relay;
            const struct sockaddr_in6 *actual =
                (const struct sockaddr_in6 *)&sender_addr;

            same_address =
                expected->sin6_port == actual->sin6_port &&
                expected->sin6_scope_id == actual->sin6_scope_id &&
                memcmp(&expected->sin6_addr,
                       &actual->sin6_addr,
                       sizeof(expected->sin6_addr)) == 0;
        }

        if (!same_address) {
            continue;
        }

        /* Relay accepted registration. */
        if (strcmp(response, "OK") == 0) {
            return 0;
        }

        /* Relay explicitly rejected registration. */
        if (strncmp(response, "ERR ", 4) == 0) {
            fprintf(stderr, "Sender registration failed: %s\n",
                    response + 4);
            return 1;
        }

        /* Unexpected response: retry. */
    }

    fprintf(stderr,
            "Sender registration failed after 5 attempts\n");

    return 1;
}

int runSender(const char *relay, const char *port, const char *session, const char *filename, uint32_t window, uint32_t timeout_ms, double loss, double corrupt, double dup)
{
    FILE *file = NULL;
    struct addrinfo *addresses = NULL;
    struct addrinfo *addr = NULL;
    struct sockaddr_storage relay_storage;
    struct sockaddr *relay_addr = (struct sockaddr *)&relay_storage;
    socklen_t relay_len = 0;
    int sockfd = -1;
    int result = 1;

    SenderState sender;
    uint8_t buffer[MAX_PACKET_SIZE];
    uint8_t file_buffer[MAX_PAYLOAD];

    if (relay == NULL || port == NULL || session == NULL ||
        filename == NULL) {
        fprintf(stderr, "runSender: invalid argument\n");
        return 1;
    }

    if (window < 1 || window > MAX_WINDOW || timeout_ms == 0) {
        fprintf(stderr, "runSender: invalid window or timeout\n");
        return 1;
    }

    /* Open the file to transfer. */
    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("fopen");
        goto cleanup;
    }

    /* Resolve the relay address. */
    if (resolveAddr(relay, port, &addresses) != 0) {
        fprintf(stderr, "runSender: failed to resolve relay\n");
        goto cleanup;
    }

    /* Find an address for which a socket can be created. */
    for (addr = addresses; addr != NULL; addr = addr->ai_next) {
        sockfd = createSocket(addr);

        if (sockfd >= 0) {
            break;
        }
    }

    // GCOVR_EXCL_START
    if (sockfd < 0 || addr == NULL) {
        fprintf(stderr, "runSender: failed to create socket\n");
        goto cleanup;
    }

    if (addr->ai_addrlen > sizeof(relay_storage)) {
        fprintf(stderr, "runSender: relay address is too large\n");
        goto cleanup;
    }
    // GCOVR_EXCL_STOP

    memset(&relay_storage, 0, sizeof(relay_storage));
    memcpy(&relay_storage, addr->ai_addr, addr->ai_addrlen);
    relay_len = (socklen_t)addr->ai_addrlen;

    /* Initialize the Go-Back-N sender state. */
    // GCOVR_EXCL_START
    if (senderInit(&sender, window, timeout_ms) != 0) {
        fprintf(stderr, "runSender: failed to initialize sender\n");
        goto cleanup;
    }
    // GCOVR_EXCL_STOP

    /*
     * Register this sender with the relay.
     * The same UDP socket is used for registration and data transfer.
     */
    if (registerSender(sockfd, relay_addr, relay_len,
                       session, loss, corrupt, dup) != 0) {
        fprintf(stderr, "runSender: relay registration failed\n");
        goto cleanup;
    }

    int input_done = 0;

    /*
     * Main Go-Back-N sending loop.
     */
    while (!senderFinished(&sender)) {
        uint64_t now_ms;
        uint64_t remaining_ms = 0;
        int timer_status;

        /*
         * Fill the sending window while there is file data remaining
         * and the window has available space.
         */
        while (!input_done &&
               sender.nextseqnum - sender.base < sender.window) {
            size_t bytes_read;
            Packet packet;
            size_t encoded_size = 0;

            bytes_read = fread(file_buffer, 1, MAX_PAYLOAD, file);

            if (bytes_read == 0) {
                if (ferror(file)) {
                    perror("fread");
                    goto cleanup;
                }

                input_done = 1;
                break;
            }

            /*
             * Construct a DATA packet and update sender state.
             * senderSendData is responsible for assigning the sequence
             * number and recording the packet for retransmission.
             */
            now_ms = getTimeMs();

            if (senderSendData(&sender, file_buffer,
                               (uint16_t)bytes_read,
                               now_ms, &packet) != 0) {
                fprintf(stderr,
                        "runSender: failed to create DATA packet\n");
                goto cleanup;
            }

            /* Encode the packet into its wire representation. */
            if (encodePacket(&packet, buffer, sizeof(buffer),
                             &encoded_size) != 0) {
                fprintf(stderr,
                        "runSender: failed to encode DATA packet\n");
                goto cleanup;
            }

            /* Send the encoded datagram to the relay. */
            ssize_t sent = sendto(sockfd, buffer, encoded_size, 0,
                                  relay_addr, relay_len);

            if (sent < 0 || (size_t)sent != encoded_size) {
                perror("sendto DATA");
                goto cleanup;
            }
        }

        /*
         * Send FIN only after the entire file has been read and every
         * DATA packet has been acknowledged.
         */
        if (input_done &&
            sender.base == sender.nextseqnum &&
            !sender.fin_sent) {
            Packet fin_packet;
            size_t encoded_size = 0;

            now_ms = getTimeMs();

            if (senderSendFin(&sender, now_ms, &fin_packet) != 0) {
                fprintf(stderr,
                        "runSender: failed to create FIN packet\n");
                goto cleanup;
            }

            if (encodePacket(&fin_packet, buffer, sizeof(buffer),
                             &encoded_size) != 0) {
                fprintf(stderr,
                        "runSender: failed to encode FIN packet\n");
                goto cleanup;
            }

            ssize_t sent = sendto(sockfd, buffer, encoded_size, 0,
                                  relay_addr, relay_len);

            if (sent < 0 || (size_t)sent != encoded_size) {
                perror("sendto FIN");
                goto cleanup;
            }
        }

        /*
         * Check whether the current timer has expired.
         *
         * Assumes senderTimerDue() returns:
         *   > 0 when the timer is due,
         *     0 when it is not due,
         *   < 0 on error.
         */
        now_ms = getTimeMs();
        timer_status = senderTimerDue(&sender, now_ms, &remaining_ms);

        if (timer_status != 0) {
            fprintf(stderr, "runSender: timer check failed\n");
            goto cleanup;
        }

        if (sender.timer_running && remaining_ms == 0) {
            /* DATA timeout: retransmit all outstanding packets. */
            if (sender.base < sender.nextseqnum) {
                Packet retransmissions[MAX_WINDOW];
                size_t retransmission_count = 0;

                if (senderHandleTimeout(&sender, now_ms,
                                        retransmissions,
                                        &retransmission_count) != 0) {
                    fprintf(stderr,
                            "runSender: DATA timeout handling failed\n");
                    goto cleanup;
                }

                for (size_t i = 0; i < retransmission_count; i++) {
                    size_t encoded_size = 0;

                    if (encodePacket(&retransmissions[i], buffer,
                                    sizeof(buffer), &encoded_size) != 0) {
                        fprintf(stderr,
                                "runSender: failed to encode retransmission\n");
                        goto cleanup;
                    }

                    ssize_t sent = sendto(sockfd, buffer, encoded_size, 0,
                                        relay_addr, relay_len);

                    if (sent < 0 || (size_t)sent != encoded_size) {
                        perror("sendto retransmission");
                        goto cleanup;
                    }
                }
            } else if (sender.fin_sent && !sender.fin_acked) {
                /* Retransmit FIN and restart its timer. */
                size_t encoded_size = 0;

                if (encodePacket(&sender.fin, buffer, sizeof(buffer),
                                &encoded_size) != 0) {
                    fprintf(stderr,
                            "runSender: failed to encode FIN retransmission\n");
                    goto cleanup;
                }

                ssize_t sent = sendto(sockfd, buffer, encoded_size, 0,
                                    relay_addr, relay_len);

                if (sent < 0 || (size_t)sent != encoded_size) {
                    perror("sendto FIN retransmission");
                    goto cleanup;
                }

                sender.timer_start = getTimeMs();
                sender.timer_running = 1;
            }

            continue;
        }

        /*
         * Wait for an incoming datagram. If a timer is running,
         * wait no longer than its remaining duration.
         */
        struct pollfd pfd;
        pfd.fd = sockfd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        int poll_timeout = -1;

        if (sender.timer_running) {
            if (remaining_ms > (uint64_t)INT_MAX) {
                poll_timeout = INT_MAX;
            } else {
                poll_timeout = (int)remaining_ms;
            }
        }

        int poll_result = poll(&pfd, 1, poll_timeout);

        if (poll_result < 0) {
            if (errno == EINTR) {
                continue;
            }

            perror("poll");
            goto cleanup;
        }

        if (poll_result == 0) {
            /* Timer expired; the next iteration handles retransmission. */
            continue;
        }

        if ((pfd.revents & POLLIN) == 0) {
            continue;
        }

        /*
         * Receive an ACK from the relay.
         */
        uint8_t received_buffer[MAX_PACKET_SIZE];
        struct sockaddr_storage source_storage;
        struct sockaddr *source_addr =
            (struct sockaddr *)&source_storage;
        socklen_t source_len = sizeof(source_storage);

        ssize_t received = recvfrom(sockfd, received_buffer,
                                    sizeof(received_buffer), 0,
                                    source_addr, &source_len);

        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }

            perror("recvfrom");
            goto cleanup;
        }

        /*
         * Ignore datagrams that did not come from the configured relay.
         * Compare address family, IP address, and UDP port.
         */
        int from_relay = 0;

        if (source_addr->sa_family == relay_addr->sa_family) {
            if (relay_addr->sa_family == AF_INET &&
                source_len >= sizeof(struct sockaddr_in) &&
                relay_len >= sizeof(struct sockaddr_in)) {
                const struct sockaddr_in *source_ipv4 =
                    (const struct sockaddr_in *)source_addr;
                const struct sockaddr_in *relay_ipv4 =
                    (const struct sockaddr_in *)relay_addr;

                from_relay =
                    source_ipv4->sin_port == relay_ipv4->sin_port &&
                    source_ipv4->sin_addr.s_addr ==
                        relay_ipv4->sin_addr.s_addr;
            } else if (relay_addr->sa_family == AF_INET6 &&
                       source_len >= sizeof(struct sockaddr_in6) &&
                       relay_len >= sizeof(struct sockaddr_in6)) {
                const struct sockaddr_in6 *source_ipv6 =
                    (const struct sockaddr_in6 *)source_addr;
                const struct sockaddr_in6 *relay_ipv6 =
                    (const struct sockaddr_in6 *)relay_addr;

                from_relay =
                    source_ipv6->sin6_port == relay_ipv6->sin6_port &&
                    memcmp(&source_ipv6->sin6_addr,
                           &relay_ipv6->sin6_addr,
                           sizeof(struct in6_addr)) == 0 &&
                    source_ipv6->sin6_scope_id ==
                        relay_ipv6->sin6_scope_id;
            }
        }

        if (!from_relay) {
            continue;
        }

        /*
         * Decode the packet. Invalid packets are discarded.
         */
        Packet received_packet;

        if (decodePacket(received_buffer, (size_t)received,
                         &received_packet) != 0) {
            continue;
        }

        if (received_packet.type != PACKET_ACK) {
            continue;
        }

        now_ms = getTimeMs();

        /*
         * Once FIN has been sent, process ACKs as FIN acknowledgements.
         * Otherwise process them as cumulative DATA acknowledgements.
         */
        if (sender.fin_sent && !sender.fin_acked) {
            if (senderHandleFinAck(&sender, received_packet.seq,
                                   now_ms) != 0) {
                /*
                 * A duplicate or unrelated ACK may not acknowledge FIN.
                 * Ignore it rather than terminating the transfer.
                 */
                continue;
            }
        } else {
            if (senderHandleAck(&sender, received_packet.seq,
                                now_ms) != 0) {
                /*
                 * Invalid, duplicate, or out-of-range ACKs should not
                 * cause the sender to terminate.
                 */
                continue;
            }
        }
    }

    /* The sender has received the final acknowledgement. */
    result = 0;

cleanup:
    if (sockfd >= 0) {
        close(sockfd);
    }

    if (addresses != NULL) {
        freeaddrinfo(addresses);
    }

    if (file != NULL) {
        fclose(file);
    }

    return result;
}

int runReceiver(const char *relay, const char *port, const char *session, const char *filename)
{
    struct addrinfo hints;
    struct addrinfo *results = NULL;
    struct addrinfo *addr;
    struct sockaddr_storage peer_addr;
    socklen_t peer_len;

    FILE *output = NULL;
    int sockfd = -1;
    int status = 1;
    int registered = 0;

    ReceiverState receiver;

    uint8_t buffer[MAX_PACKET_SIZE];
    uint8_t encoded[MAX_PACKET_SIZE];

    size_t encoded_size;
    ssize_t received;
    ssize_t sent;

    uint64_t now_ms;
    uint64_t linger_start = 0;
    const uint64_t linger_ms = 2000;

    if (relay == NULL || port == NULL ||
        session == NULL || filename == NULL) {
        return 1;
    }

    /* Open the destination file for binary output. */
    output = fopen(filename, "wb");
    if (output == NULL) {
        perror("fopen");
        goto cleanup;
    }

    /* Resolve the relay address. */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    if (resolveAddr(relay, port, &results) != 0) {
        fprintf(stderr, "Could not resolve relay address\n");
        goto cleanup;
    }

    /*
     * Create one UDP socket and register with the relay.
     * Keep this socket for the entire transfer.
     */
    for (addr = results; addr != NULL; addr = addr->ai_next) {
        sockfd = createSocket(addr);
        if (sockfd < 0) {
            continue;
        }

        if (registerReceiver(sockfd, addr->ai_addr,
                             (socklen_t)addr->ai_addrlen,
                             session) == 0) {
            registered = 1;
            break;
        }

        close(sockfd);
        sockfd = -1;
    }

    if (!registered) {
        fprintf(stderr, "Receiver registration failed\n");
        goto cleanup;
    }

    /* Initialize the receiver state machine. */
    if (receiverInit(&receiver) != 0) {
        fprintf(stderr, "Could not initialize receiver\n");
        goto cleanup;
    }

    /*
     * Receive DATA and FIN packets until the transfer completes.
     * Invalid datagrams are silently discarded.
     */
    while (!receiver.finished) {
        struct pollfd pfd;
        Packet packet;
        Packet ack;
        int poll_result;
        uint32_t previous_expected;
        int accepted_data;

        pfd.fd = sockfd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        poll_result = poll(&pfd, 1, 1000);

        if (poll_result < 0) {
            if (errno == EINTR) {
                continue;
            }

            perror("poll");
            goto cleanup;
        }

        if (poll_result == 0) {
            now_ms = getTimeMs();

            if (receiverTimerExpired(&receiver, now_ms)) {
                fprintf(stderr, "Receiver timed out\n");
                goto cleanup;
            }

            continue;
        }

        if (!(pfd.revents & POLLIN)) {
            if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
                fprintf(stderr, "UDP socket error\n");
                goto cleanup;
            }

            continue;
        }

        peer_len = sizeof(peer_addr);

        received = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                            (struct sockaddr *)&peer_addr,
                            &peer_len);

        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }

            perror("recvfrom");
            goto cleanup;
        }

        /*
         * Validate the actual datagram length, header fields,
         * payload length, packet type, and checksum before
         * processing or copying any payload.
         */
        if (decodePacket(buffer, (size_t)received, &packet) != 0) {
            continue;
        }

        now_ms = getTimeMs();

        /*
         * Save the expected sequence number before processing
         * DATA. Only the packet that was expected beforehand
         * may be written to the output file.
         */
        previous_expected = receiver.expectedseqnum;
        accepted_data = 0;

        if (packet.type == PACKET_DATA) {
            if (receiver.finished) {
                continue;
            }

            accepted_data = (packet.seq == previous_expected);

            if (receiverHandleData(&receiver, &packet,
                                   now_ms, &ack) != 0) {
                fprintf(stderr, "Could not process DATA packet\n");
                goto cleanup;
            }

            if (accepted_data && packet.length > 0) {
                if (fwrite(packet.payload, 1, packet.length,
                           output) != packet.length) {
                    perror("fwrite");
                    goto cleanup;
                }
            }
        } else if (packet.type == PACKET_FIN) {
            /*
             * The state-machine function determines whether FIN
             * is expected and creates the appropriate ACK.
             */
            if (receiverHandleFin(&receiver, &packet,
                                 now_ms, &ack) != 0) {
                fprintf(stderr, "Could not process FIN packet\n");
                goto cleanup;
            }

            if (receiver.finished && linger_start == 0) {
                linger_start = getTimeMs();

                if (fflush(output) != 0) {
                    perror("fflush");
                    goto cleanup;
                }
            }
        } else {
            /*
             * ACK packets are not expected by the receiver.
             */
            continue;
        }

        /*
         * Encode and send the cumulative ACK through the relay.
         * The ACK sequence number comes from the state machine.
         */
        if (encodePacket(&ack, encoded, sizeof(encoded),
                         &encoded_size) != 0) {
            fprintf(stderr, "Could not encode ACK packet\n");
            goto cleanup;
        }

        sent = sendto(sockfd, encoded, encoded_size, 0,
                      (struct sockaddr *)&peer_addr, peer_len);

        if (sent < 0 || (size_t)sent != encoded_size) {
            if (sent < 0) {
                perror("sendto");
            } else {
                fprintf(stderr, "Incomplete UDP send\n");
            }

            goto cleanup;
        }
    }

    /*
     * FIN was accepted. Linger briefly to ACK retransmitted FIN
     * packets if the original final ACK was lost by the relay.
     */
    while (getTimeMs() - linger_start < linger_ms) {
        struct pollfd pfd;
        Packet packet;
        Packet ack;
        int poll_result;

        uint64_t elapsed = getTimeMs() - linger_start;
        int remaining = (int)(linger_ms - elapsed);

        pfd.fd = sockfd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        poll_result = poll(&pfd, 1, remaining);

        if (poll_result < 0) {
            if (errno == EINTR) {
                continue;
            }

            perror("poll during FIN linger");
            goto cleanup;
        }

        if (poll_result == 0) {
            break;
        }

        if (!(pfd.revents & POLLIN)) {
            continue;
        }

        peer_len = sizeof(peer_addr);

        received = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                            (struct sockaddr *)&peer_addr,
                            &peer_len);

        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }

            perror("recvfrom during FIN linger");
            goto cleanup;
        }

        if (decodePacket(buffer, (size_t)received, &packet) != 0) {
            continue;
        }

        /*
         * A repeated FIN should receive the final cumulative ACK
         * again. Ignore other packets during the linger period.
         */
        if (packet.type != PACKET_FIN) {
            continue;
        }

        if (receiverHandleFin(&receiver, &packet,
                              getTimeMs(), &ack) != 0) {
            continue;
        }

        if (encodePacket(&ack, encoded, sizeof(encoded),
                         &encoded_size) != 0) {
            continue;
        }

        sent = sendto(sockfd, encoded, encoded_size, 0,
                      (struct sockaddr *)&peer_addr, peer_len);

        if (sent < 0 || (size_t)sent != encoded_size) {
            if (sent < 0 && errno != EINTR) {
                perror("sendto during FIN linger");
                goto cleanup;
            }
        }
    }

    if (fflush(output) != 0) {
        perror("fflush");
        goto cleanup;
    }

    status = 0;

cleanup:
    if (results != NULL) {
        freeaddrinfo(results);
    }

    if (sockfd >= 0) {
        close(sockfd);
    }

    if (output != NULL) {
        if (fclose(output) != 0) {
            perror("fclose");
            status = 1;
        }
    }

    return status;
}

void printUsage(void)
{
    printf("Usage: myapp send -s <session> [-w window] [-T timeout-ms] [-l loss]\n"
           "                  [-c corrupt] [-d dup] [-p port] <relay> <file>\n"
           "       myapp recv -s <session> [-p port] <relay> <file>\n"
           "\n"
           "  -s <session>     session name shared by the sender and the receiver\n"
           "  -w <window>      Go-Back-N window size in packets, 1 to 64 (default: 8)\n"
           "  -T <timeout-ms>  retransmission timeout in milliseconds (default: 250)\n"
           "  -l <loss>        probability the relay drops a packet (default: 0)\n"
           "  -c <corrupt>     probability the relay flips a bit (default: 0)\n"
           "  -d <dup>         probability the relay duplicates a packet (default: 0)\n"
           "  -p <port>        relay port (default: 4250)\n"
           "  <relay>          host name or address of the relay\n"
           "  <file>           file to send, or file to write what is received\n");
}


```

### lab.h

```c

#ifndef LAB_H
#define LAB_H

#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX_PAYLOAD 1024
#define HEADER_SIZE 10
#define MAX_PACKET_SIZE 1034
#define MAX_WINDOW 64

typedef enum {
    PACKET_DATA = 0,
    PACKET_ACK = 1,
    PACKET_FIN = 2
} PacketType;

typedef struct {
    uint8_t type;
    uint32_t seq;
    uint16_t length;
    uint8_t payload[MAX_PAYLOAD];
} Packet;

typedef struct {
    uint32_t base;
    uint32_t nextseqnum;
    uint32_t window;
    uint32_t timeout_ms;
    uint32_t consecutive_timeouts;
    uint64_t timer_start;
    int timer_running;

    Packet packets[MAX_WINDOW];
    Packet fin;
    int fin_sent;
    int fin_acked;
} SenderState;

typedef struct {
    uint32_t expectedseqnum;
    int finished;
    uint64_t last_activity;
} ReceiverState;

/* Packet functions */

/** * @brief calculates the internet checksum of a packet buffer
*
* Treats the buffer as 16-bit words and calculates the checksum.
* If the buffer has an odd number of bytes a zero byte is added for calculation.
*
* @param data buffer containing the data to checksum
* @param length the number of bytes in the buffer
* @return the 16-bit internet checksum
*/
uint16_t packetChecksum(const uint8_t *data, size_t length);

/** * @brief encodes a packet to be able to send over the network
*
* Writes the packet header and payload into the buffer using network byte 
* order for all multi-byte fields. The checksum is calculated and placed
* inot the encoded packet.
*
* @param packet the packet structure to encode 
* @param buffer the buffer that will contain the encoded packet
* @param buffer_size size of the output buffer
* @param packet_size pointer to where the encoded packet will be stored
* @return 0 on success nonzero on failure
 */
int encodePacket(const Packet *packet, uint8_t *buffer, size_t buffer_size, size_t *packet_size);

/** *@brief decodes and validates a recieved packet
*
* Checks the packet size, type, reserved field, payload length, and 
* checksum before copying decoded fields into packet structure.
* Invalid packets are rejected without producing a valid packet.
*
* @param buffer the buffer containing the recieved datagram
* @param buffer_size number of bytes actually received
* @param packet the packet structure where the decoded packet will be stored
* @return 0 on success nonzero if the packet is invalid
 */
int decodePacket(const uint8_t *buffer, size_t buffer_size, Packet *packet);

/** *@brief creates a data packet
*
* Initializes a packet structure as a data packet with the 
* specified sequence number and payload
*
* @param packet the packet structure to initialize
* @param seq the sequence number of the data packet
* @param data the payload to place in the datapacket
* @param length the number of payload bytes
* @return 0 on success, nonzero if the arguments or payload length are invalid
 */
int makeDataPacket(Packet *packet, uint32_t seq, const uint8_t *data, uint16_t length);

/** @brief creates an ack packet
*
* Initializes a packet strucure as an ack containing the specified
* cumulative acknowledgement sequence number
*
* @param packet the packet structure to initialize
* @param seq the sequence number of the next packet expected by reciever
* @return 0 on success, nonzero of the packet pointer is invalid
 */
int makeAckPacket(Packet *packet, uint32_t seq);

/** *@brief creates a fin packet
*
* Initializes a packet structure as a fin packet with
* the specified sequence number and payload
*
* @param packet the packet structure to initialize
* @param seq the sequence number of the fin packet
* @return 0 on success, nonzero if the packet pointer is invalid
*/
int makeFinPacket(Packet *packet, uint32_t seq);



/* Sender state machine */

/** *@brief initializes the sender go-back-n state machine
*
* Sets the sender's base and next sequence number to zero, initializes
* the configured window and timeout values, clears the retransmission
* state and prepares the sender to trasmit data packets
*
* @param sender the sender state structure to init
* @param window the max number of packets allowed in the send window
* @param timeout_ms the retransmission timeout in ms
* @return 0 on success nonzero if args are invalid
 */
int senderInit(SenderState *sender, uint32_t window, uint32_t timeout_ms);

/** *@brief processes new data for transmission by snder
*
* creates the next data packet when a space is available in the sender's
* window, stores a copy for possible retransmission, advances nextseqnum,
* and starts the timer when the first unacked packet enters the window.
* the function does not send the packet itself
*
* @param sender the sender state machine
* @param data payload data for the new data packet
* @param length the number of payload bites
* @param now_ms current time in ms
* @param packet the structure where the packet to transmit will be stored
* @return 0 on success nonzero if window is full or data is invalid
 */
int senderSendData(SenderState *sender, const uint8_t *data, uint16_t length, uint64_t now_ms, Packet *packet);

/** * @brief processes a cumulative ack recieved by the sender
*
* advances base when the ack acknowledges packets that are previously
* unacknowledged. duplicate or older acks are ignored. the retransmission 
* timer is stopped when all outstanding packets are acknowledged and is
* restarted when unacknowledged packets remain.
*
* @param sender the sender state machine
* @param ack_seq the sequence number contained in the recieved ack
* @param now_ms the current time in ms
* @return 0 if ack was processed, nonzero on error
 */
int senderHandleAck(SenderState *sender, uint32_t ack_seq, uint64_t now_ms);

/** * @brief processes a sender retransmission timeout
*
* resets the retransmission timer and produces copies of every unacknowledged packet
* from base through nextseqnum - 1 so that the caller can retransmit the entire go-back-n window
*
* @param sender the sender state machine
* @param now_ms the current time in ms
* @param packets array where the packets requiring restransmission will be stored
* @param count pointer where the number of packets to retransmit will be stored
* @return 0 on success, nonzero if the sender must give up
 */
int senderHandleTimeout(SenderState *sender, uint64_t now_ms, Packet *packets, size_t *count);

/** * @brief determines how much time remains on the sender's timer
*
* calculates the time remaining before the retransmission timer expires
*
* @param sender the sender state machine
* @param now_ms curren time in ms
* @param remaining_ms pointer where the remaining timer duration will be stored
* @return 0 if a timer is active, nonzero if no timer is running
 */
int senderTimerDue(const SenderState *sender, uint64_t now_ms, uint64_t *remaining_ms);

/** * @brief creates the sender's fin packet after all data is acknowledged
*
* creates the fin packet using the next sequence number, records that fin has been
* sent, and starts the retransmission timer
*
* @param sender the sender state machine
* @param now_ms the current time in ms
* @param packet the structure where the fin packet will be stored
* @return 0 on success, nonzero if data packets remain unacknowledged
 */
int senderSendFin(SenderState *sender, uint64_t now_ms, Packet *packet);

/** * @brief process the ack for the sender's fin packet
*
* marks the fin as acknowledged and stops the retransmission timer when the 
* recieved ack acknowledges the fin
*
* @param sender the sender state machine
* @param ack_seq sequence number contained in the recieved ack
* @param now_ms the current time in ms
* @return 0 if the fin was acknowledged, nonzero otherwise
 */
int senderHandleFinAck(SenderState *sender, uint32_t ack_seq, uint64_t now_ms);

/** * @brief determines whether the sender has completed the transfer
*
* checks whether all data packets and the fin packet has been acknowledged
*
* @param sender the sender state machine
* @return nonzero if the transfer is complete, otherwise zero
 */
int senderFinished(const SenderState *sender);



/* Reciever state machine */

/** * @brief initializes the receiver go-back-n state machine
*
* sets the receiver's expected sequence number to 0 and initializes
* its transfer and activity state
*
* @param receiver the receiver state machine to initialize
* @param return 0 for success, nonzero value if receiver is null
 */
int receiverInit(ReceiverState *receiver);

/** * @brief processes a data packet recieved by the receiver
*
* accepts and delivers the packet when its sequence number matches expectedseqnum
* an accepted packet advances expectedseqnum and produces a cumulative ack. duplicate
* and out of order packets are discarded and cause an ack for the current expected
* sequence number to be produced
*
* @param receiver the receiver state machine
* @param packet the recieved data packet
* @param now_ms the current time in ms
* @param ack the structure where the ack to send will be stored
 */
int receiverHandleData(ReceiverState *receiver, const Packet *packet, uint64_t now_ms, Packet *ack);
 
/** * @brief processes a fin packet received by the reciever 
*
* accepts the fin when its sequence number matches expetedseqnum, advances
* the expected sequence number, creates the final ack, and marks the reciever
* as finished. a repeated fin is answered with the same cumulative ack
* during the reciever's linger period
*
* @param reciever the reciever state machine
* @param packet the recieved fin packet
* @param now_ms the current time in ms
* @param ack structure where the ack to send will be stored
* @return 0 on success, nonzero on error
 */
int receiverHandleFin(ReceiverState *receiver, const Packet *packet, uint64_t now_ms, Packet *ack);

/** * @brief determines whether the recevier has exceeded its inactivity timeout
*
* compared the supplied current time with the receiver's last activity time to 
* determine whether 30 seconds have elapsed without a valid packet
*
* @param receiver the receiver state machine
* @param now_ms the curren time in ms
* @return nonzero if the receiver has timed out, otherwise zero 
 */
int receiverTimerExpired(const ReceiverState *receiver, uint64_t now_ms);



/* I/O */

/** * @brief gets the current time in milliseconds
*
* uses CLOCK_MONOTONIC to botain a clock value that does not change when the 
* system wall clock is adjusted
*
* @return current time in ms
 */
uint64_t getTimeMs(void);

/** * @brief resolves a relay hostname or address into socket addresses
*
* uses getaddrinfo to resolve the supplied relay host and port so that the caller
* can connect without assuming the relay uses a particular address format
*
* @param host the relay hostname or address
* @param port the relay port as a string
* @param results pointer where the resulting address list will be stored
* @return 0 on success, nonzero if address resolution fails
 */
int resolveAddr(const char *host, const char *port, struct addrinfo **results);

/** * @brief creates a udp socket for a resolved address
*
* creates a udp socket using the address family and protocol information contained 
* in the supplied addrinfo structure
*
* @param addr resolved address information used to create the socket
* @return socket file descriptor on success, or -1 on failure
 */
int createSocket(const struct addrinfo *addr);

/** * @brief registers the reciever with the relay
*
* sends the receiver hello message to the relay and waits for an ok response.
* if no response is attempted within one second the hello retried up to 5 times
*
* @param sockfd UDP socket used for the transfer
* @param relay address of the relay
* @param relay_len length of the relay address structure
* @param session the session name shared with the sender
* @return 0 when registration succeeds, nonzero when registration fails
 */
int registerReceiver(int sockfd, const struct sockaddr *relay, socklen_t relay_len, const char *session);

/** * @brief registers the sender with the sender with the relay
* 
* sends the sender hello message containing the session name and packet loss,
* corruption, and duplication probabilities, then waits for an OK response. if no 
* response is recieved within one second, hello is retried up to 5 times
*
* @param sockfd UDP socket used for the transfer
* @param relay address of the relay
* @param relay_len the legnth of the relay address structure
* @param session the session name shared with the receiver
* @param loss probability that the relay drops a packet
* @param corrupt the probability that the relay corrupts a packet
* @param dup the probability that the relay duplicates a packet
* @return 0 when registration succeeds, nonzeor when it fails
 */
int registerSender(int sockfd, const struct sockaddr *relay, socklen_t relay_len, const char *session, double loss, double corrupt, double dup);

/** * @brief runs the complete sender transfer
*
* opens the input file, registers the sender with the relay, drives the sender
* state machine using UDP and poll, transmits the data and fin packets, handles 
* cumulative acks and retransmission timeouts, and closes all resources when 
* the transfer completes or fails
*
* @param relay relay hostname or address
* @param port relay udp port
* @param session session name shared with the reciever
* @param filename input file to transmit
* @param window go-back-n window size in packets
* @param timeout_ms retransmission timeout in ms
* @param loss packet loss probability requested from the relay
* @param corrupt packet corruption probability requested form the relay
* @param dup packet duplication probability requested from the relay
* @return 0 on successful transfer, nonzero on network or transfer failure
 */
int runSender(const char *relay, const char *port, const char *session, const char *filename, uint32_t window, uint32_t timeout_ms, double loss, 
            double corrupt, double dup);

/** * @brief runs the complete reciever transfer
*
* opens the output file, registers the receiver with the relay, recieves and 
* validates packets, drives the receiver state machine, writes accepted data payloads
* in order, sends cumulative acks, and lingers after fin to handle retransmitted fin packets
*
* @param relay the relay hostname or address
* @param port relay udp port
* @param session session name shared with the sender
* @param filename output file where recieved data will be written
* @return 0 on succesful transfer, nonzero on network or transfer failure
 */            
int runReceiver(const char *relay, const char *port, const char *session, const char *filename);

/* Command line */

/** * @brief pritns the program's command-line usage info
*
* prints the required send and recieve command formats along with the available command line options and their default values
*
* @return none
 */
void printUsage(void);

#endif 
```

### main.c

```c


#define _POSIX_C_SOURCE 200809L

#include "lab.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef TEST
#define main main_exclude
#endif

int main(int argc, char *argv[])
{
    const char *mode;
    const char *session = NULL;
    const char *relay;
    const char *filename;
    const char *port = "4250";

    char port_buffer[6];

    uint32_t window = 8U;
    uint32_t timeout_ms = 250U;

    double loss = 0.0;
    double corrupt = 0.0;
    double dup = 0.0;

    int index = 2;
    int result;

    /* No arguments: print usage and exit successfully. */
    if (argc == 1) {
        printUsage();
        return 0;
    }

    /* Validate the mode. */
    if (argc < 2 ||
        (strcmp(argv[1], "send") != 0 &&
         strcmp(argv[1], "recv") != 0)) {
        fprintf(stderr, "Error: mode must be 'send' or 'recv'.\n");
        printUsage();
        return 1;
    }

    mode = argv[1];

    /* Parse command-line options. */
    while (index < argc && argv[index][0] == '-') {
        const char *option = argv[index];

        if (strcmp(option, "-s") == 0) {
            if (index + 1 >= argc || session != NULL) {
                fprintf(stderr, "Error: invalid -s option.\n");
                printUsage();
                return 1;
            }

            session = argv[index + 1];
            index += 2;
        } else if (strcmp(option, "-p") == 0) {
            char *end = NULL;
            unsigned long value;

            if (index + 1 >= argc || argv[index + 1][0] == '\0') {
                fprintf(stderr, "Error: -p requires a port.\n");
                printUsage();
                return 1;
            }

            errno = 0;
            value = strtoul(argv[index + 1], &end, 10);

            if (errno != 0 || end == argv[index + 1] ||
                *end != '\0' || value < 1UL || value > 65535UL) {
                fprintf(stderr, "Error: invalid port.\n");
                printUsage();
                return 1;
            }

            (void)snprintf(port_buffer, sizeof(port_buffer),
                           "%lu", value);
            port = port_buffer;
            index += 2;
        } else if (strcmp(mode, "send") == 0 &&
                   strcmp(option, "-w") == 0) {
            char *end = NULL;
            unsigned long value;

            if (index + 1 >= argc) {
                fprintf(stderr, "Error: -w requires a window size.\n");
                printUsage();
                return 1;
            }

            errno = 0;
            value = strtoul(argv[index + 1], &end, 10);

            if (errno != 0 || end == argv[index + 1] ||
                *end != '\0' || value < 1UL ||
                value > (unsigned long)MAX_WINDOW) {
                fprintf(stderr, "Error: window must be between 1 and %u.\n",
                        (unsigned int)MAX_WINDOW);
                printUsage();
                return 1;
            }

            window = (uint32_t)value;
            index += 2;
        } else if (strcmp(mode, "send") == 0 &&
                   strcmp(option, "-T") == 0) {
            char *end = NULL;
            unsigned long value;

            if (index + 1 >= argc) {
                fprintf(stderr, "Error: -T requires a timeout.\n");
                printUsage();
                return 1;
            }

            errno = 0;
            value = strtoul(argv[index + 1], &end, 10);

            if (errno != 0 || end == argv[index + 1] ||
                *end != '\0' || value == 0UL ||
                value > UINT32_MAX) {
                fprintf(stderr, "Error: timeout must be positive.\n");
                printUsage();
                return 1;
            }

            timeout_ms = (uint32_t)value;
            index += 2;
        } else if (strcmp(mode, "send") == 0 &&
                   (strcmp(option, "-l") == 0 ||
                    strcmp(option, "-c") == 0 ||
                    strcmp(option, "-d") == 0)) {
            char *end = NULL;
            double value;

            if (index + 1 >= argc || argv[index + 1][0] == '\0') {
                fprintf(stderr, "Error: %s requires a probability.\n",
                        option);
                printUsage();
                return 1;
            }

            errno = 0;
            value = strtod(argv[index + 1], &end);

            if (errno != 0 || end == argv[index + 1] ||
                *end != '\0' || value < 0.0 || value > 0.5) {
                fprintf(stderr,
                        "Error: %s probability must be between 0 and 0.5.\n",
                        option);
                printUsage();
                return 1;
            }

            if (strcmp(option, "-l") == 0) {
                loss = value;
            } else if (strcmp(option, "-c") == 0) {
                corrupt = value;
            } else {
                dup = value;
            }

            index += 2;
        } else {
            fprintf(stderr, "Error: unknown option '%s'.\n", option);
            printUsage();
            return 1;
        }
    }

    /* Validate the required session name. */
    if (session == NULL) {
        fprintf(stderr, "Error: -s is required.\n");
        printUsage();
        return 1;
    }

    {
        size_t length = strlen(session);

        if (length < 1U || length > 32U) {
            fprintf(stderr, "Error: invalid session length.\n");
            return 1;
        }

        for (size_t i = 0; i < length; i++) {
            char c = session[i];

            if (!((c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') || c == '-')) {
                fprintf(stderr, "Error: invalid session characters.\n");
                return 1;
            }
        }
    }

    /* Exactly two positional arguments must remain. */
    if (argc - index != 2) {
        fprintf(stderr, "Error: expected relay and filename.\n");
        printUsage();
        return 1;
    }

    relay = argv[index];
    filename = argv[index + 1];

    if (relay[0] == '\0' || filename[0] == '\0') {
        fprintf(stderr, "Error: relay and filename cannot be empty.\n");
        return 1;
    }

    /*
     * Start the requested transfer.
     * The run functions return zero on success and nonzero on failure.
     */
    if (strcmp(mode, "send") == 0) {
        result = runSender(relay, port, session, filename,
                           window, timeout_ms, loss, corrupt, dup);
    } else {
        result = runReceiver(relay, port, session, filename);
    }

    if (result != 0) {
        fprintf(stderr, "%s transfer failed.\n",
                strcmp(mode, "send") == 0 ? "Sender" : "Receiver");
        return 2;
    }

    return 0;
}

```

## Tests Files
### lab-test.c

```c

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include "harness/unity.h"
#include "../src/lab.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <poll.h>
#include <stdint.h>

#define TEST_PORT_SIZE 16
#define TEST_TIMEOUT_MS 3000

static int test_relay_fd = -1;
static pid_t test_receiver_pid = -1;
static struct sockaddr_in test_receiver_addr;
static char test_output_path[128];

static int relay_fd;
static struct sockaddr_in relay_addr;

/* Test relay state for runSender */
static pid_t relay_pid = -1;
static char relay_port[16];


/*
 * Stop the child process and release test resources.
 */
static void testStopReceiver(void)
{
    if (test_receiver_pid > 0) {
        int status;
        pid_t result;

        /*
         * Give the receiver time to finish its FIN linger period.
         * Kill it if a test failed before completing the transfer.
         */
        result = waitpid(test_receiver_pid, &status, WNOHANG);

        if (result == 0) {
            kill(test_receiver_pid, SIGTERM);
            (void)waitpid(test_receiver_pid, &status, 0);
        }

        test_receiver_pid = -1;
    }

    if (test_relay_fd >= 0) {
        close(test_relay_fd);
        test_relay_fd = -1;
    }

    if (test_output_path[0] != '\0') {
        unlink(test_output_path);
        test_output_path[0] = '\0';
    }
}

void setUp(void)
{
    relay_fd = -1;
    relay_pid = -1;
    memset(&relay_addr, 0, sizeof(relay_addr));

    relay_port[0] = '\0';

    test_relay_fd = -1;
    test_receiver_pid = -1;
    test_output_path[0] = '\0';
    memset(&test_receiver_addr, 0, sizeof(test_receiver_addr));
}

void tearDown(void)
{
    /* Close the relay socket if it is open. */
    if (relay_fd >= 0) {
        close(relay_fd);
        relay_fd = -1;
    }

    /* Stop and reap the relay child process. */
    if (relay_pid > 0) {
        int status;

        /* Ask the child to terminate. */
        if (kill(relay_pid, SIGTERM) < 0 && errno != ESRCH) {
            perror("kill");
        }

        /* Reap the child, retrying if interrupted. */
        while (waitpid(relay_pid, &status, 0) < 0) {
            if (errno == EINTR) {
                continue;
            }

            if (errno != ECHILD) {
                perror("waitpid");
            }

            break;
        }

        relay_pid = -1;
    }

    testStopReceiver();
}

//packetChecksum

void test_packetChecksum_empty(void)
{
    uint8_t data[] = {};

    TEST_ASSERT_EQUAL_HEX16(0xffff, packetChecksum(data, 0));
}


void test_packetChecksum_singleByte(void)
{
    uint8_t data[] = {0x12};

    /* Calculation: 0x1200
     * Complement: 0xedff
     */
    TEST_ASSERT_EQUAL_HEX16(0xedff, packetChecksum(data, 1));
}


void test_packetChecksum_twoBytes(void)
{
    uint8_t data[] = {0x12, 0x34};

    /* Calculation: 0x1234
     * Complement: 0xedcb
     */
    TEST_ASSERT_EQUAL_HEX16(0xedcb, packetChecksum(data, 2));
}


void test_packetChecksum_fourBytes(void)
{
    uint8_t data[] = {
        0x12, 0x34,
        0x56, 0x78
    };

    /* 0x1234 + 0x5678 = 0x68ac
     * Complement = 0x9753
     */
    TEST_ASSERT_EQUAL_HEX16(0x9753, packetChecksum(data, 4));
}


void test_packetChecksum_oddLength(void)
{
    uint8_t data[] = {
        0x12, 0x34, 0x56
    };

    /* Calculation:
     *   0x1234 + 0x5600 = 0x6834
     *
     * Complement:
     *   0x97cb
     */
    TEST_ASSERT_EQUAL_HEX16(0x97cb, packetChecksum(data, 3));
}

// makeDataPacket

void test_makeDataPacket_validPacket(void)
{
    Packet packet;
    uint8_t data[] = {0x10, 0x20, 0x30, 0x40};
    uint32_t seq = 5;
    uint16_t length = sizeof(data);

    int result = makeDataPacket(&packet, seq, data, length);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, packet.type);
    TEST_ASSERT_EQUAL_UINT32(seq, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(length, packet.length);
    TEST_ASSERT_EQUAL_MEMORY(data, packet.payload, length);
}

void test_makeDataPacket_emptyPayload(void)
{
    Packet packet;

    int result = makeDataPacket(&packet, 0, NULL, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, packet.type);
    TEST_ASSERT_EQUAL_UINT32(0, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeDataPacket_maxPayload(void)
{
    Packet packet;
    uint8_t data[MAX_PAYLOAD];

    memset(data, 0xAB, sizeof(data));

    int result = makeDataPacket(&packet, 10, data, MAX_PAYLOAD);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, packet.type);
    TEST_ASSERT_EQUAL_UINT32(10, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(MAX_PAYLOAD, packet.length);
    TEST_ASSERT_EQUAL_MEMORY(data, packet.payload, MAX_PAYLOAD);
}

void test_makeDataPacket_nullPacket(void)
{
    uint8_t data[] = {0x01, 0x02};

    int result = makeDataPacket(NULL, 0, data, sizeof(data));

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_makeDataPacket_nullDataWithNonzeroLength(void)
{
    Packet packet;

    int result = makeDataPacket(&packet, 0, NULL, 1);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_makeDataPacket_payloadTooLarge(void)
{
    Packet packet;
    uint8_t data[MAX_PAYLOAD + 1];

    int result = makeDataPacket(&packet, 0, data,
                                MAX_PAYLOAD + 1);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_makeDataPacket_zeroLengthWithNonNullData(void)
{
    Packet packet;
    uint8_t data[] = {0x55};

    int result = makeDataPacket(&packet, 7, data, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, packet.type);
    TEST_ASSERT_EQUAL_UINT32(7, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}


//test makeAckPacket

void test_makeAckPacket_validPacket(void)
{
    Packet packet;

    int result = makeAckPacket(&packet, 5);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, packet.type);
    TEST_ASSERT_EQUAL_UINT32(5, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeAckPacket_zeroSequenceNumber(void)
{
    Packet packet;

    int result = makeAckPacket(&packet, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, packet.type);
    TEST_ASSERT_EQUAL_UINT32(0, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeAckPacket_maxSequenceNumber(void)
{
    Packet packet;

    int result = makeAckPacket(&packet, UINT32_MAX);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, packet.type);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeAckPacket_nullPacket(void)
{
    int result = makeAckPacket(NULL, 5);

    TEST_ASSERT_NOT_EQUAL(0, result);
}


//test makeFinPacket

void test_makeFinPacket_validPacket(void)
{
    Packet packet;

    int result = makeFinPacket(&packet, 10);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_FIN, packet.type);
    TEST_ASSERT_EQUAL_UINT32(10, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeFinPacket_zeroSequenceNumber(void)
{
    Packet packet;

    int result = makeFinPacket(&packet, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_FIN, packet.type);
    TEST_ASSERT_EQUAL_UINT32(0, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeFinPacket_maxSequenceNumber(void)
{
    Packet packet;

    int result = makeFinPacket(&packet, UINT32_MAX);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_FIN, packet.type);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeFinPacket_nullPacket(void)
{
    int result = makeFinPacket(NULL, 10);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

// test encodePacket

void test_encodePacket_dataPacket(void)
{
    Packet packet;
    uint8_t data[] = {0x11, 0x22, 0x33};
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 5, data, sizeof(data)));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT16(HEADER_SIZE + sizeof(data), packet_size);

    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, buffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0, buffer[1]);

    uint32_t seq;
    uint16_t length;

    memcpy(&seq, &buffer[4], sizeof(seq));
    memcpy(&length, &buffer[8], sizeof(length));

    TEST_ASSERT_EQUAL_UINT32(5, ntohl(seq));
    TEST_ASSERT_EQUAL_UINT16(sizeof(data), ntohs(length));

    TEST_ASSERT_EQUAL_MEMORY(data, &buffer[HEADER_SIZE], sizeof(data));
}

void test_encodePacket_ackPacket(void)
{
    Packet packet;
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&packet, 5));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT16(HEADER_SIZE, packet_size);

    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, buffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0, buffer[1]);

    uint32_t seq;
    uint16_t length;

    memcpy(&seq, &buffer[4], sizeof(seq));
    memcpy(&length, &buffer[8], sizeof(length));

    TEST_ASSERT_EQUAL_UINT32(5, ntohl(seq));
    TEST_ASSERT_EQUAL_UINT16(0, ntohs(length));
}

void test_encodePacket_finPacket(void)
{
    Packet packet;
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 10));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT16(HEADER_SIZE, packet_size);

    TEST_ASSERT_EQUAL_UINT8(PACKET_FIN, buffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0, buffer[1]);

    uint32_t seq;
    uint16_t length;

    memcpy(&seq, &buffer[4], sizeof(seq));
    memcpy(&length, &buffer[8], sizeof(length));

    TEST_ASSERT_EQUAL_UINT32(10, ntohl(seq));
    TEST_ASSERT_EQUAL_UINT16(0, ntohs(length));
}

void test_encodePacket_emptyDataPacket(void)
{
    Packet packet;
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    TEST_ASSERT_EQUAL_INT(0, makeDataPacket(&packet, 0, NULL, 0));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT16(HEADER_SIZE, packet_size);
    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, buffer[0]);
}

void test_encodePacket_maxPayload(void)
{
    Packet packet;
    uint8_t data[MAX_PAYLOAD];
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    memset(data, 0xAB, sizeof(data));

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 7, data, MAX_PAYLOAD));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT16(MAX_PACKET_SIZE, packet_size);
    TEST_ASSERT_EQUAL_MEMORY(data, &buffer[HEADER_SIZE], MAX_PAYLOAD);
}

void test_encodePacket_nullPacket(void)
{
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    int result = encodePacket(
        NULL, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_nullBuffer(void)
{
    Packet packet;
    size_t packet_size = 0;

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&packet, 0));

    int result = encodePacket(
        &packet, NULL, MAX_PACKET_SIZE, &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_nullPacketSize(void)
{
    Packet packet;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&packet, 0));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), NULL);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_bufferTooSmall(void)
{
    Packet packet;
    uint8_t data[] = {0xAA, 0xBB, 0xCC};
    uint8_t buffer[HEADER_SIZE + sizeof(data) - 1];
    size_t packet_size = 0;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, data, sizeof(data)));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_invalidType(void)
{
    Packet packet = {0};
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    packet.type = 99;
    packet.seq = 0;
    packet.length = 0;

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_ackWithPayload(void)
{
    Packet packet = {0};
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    packet.type = PACKET_ACK;
    packet.seq = 1;
    packet.length = 1;
    packet.payload[0] = 0xAA;

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_finWithPayload(void)
{
    Packet packet = {0};
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    packet.type = PACKET_FIN;
    packet.seq = 1;
    packet.length = 1;
    packet.payload[0] = 0xAA;

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_payloadTooLarge(void)
{
    Packet packet = {0};
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    packet.type = PACKET_DATA;
    packet.seq = 0;
    packet.length = MAX_PAYLOAD + 1;

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}


//test decodePacket

/* Helper: encode a packet into a valid wire-format buffer. */
static size_t makeEncodedTestPacket(
    Packet *source, uint8_t *buffer, size_t buffer_size)
{
    size_t packet_size = 0;

    int result = encodePacket(
        source, buffer, buffer_size, &packet_size);

    if (result != 0) {
        return 0;
    }

    return packet_size;
}

void test_decodePacket_validDataPacket(void)
{
    Packet original;
    Packet decoded;
    uint8_t data[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&original, 5, data, sizeof(data)));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL_UINT32(HEADER_SIZE + sizeof(data), size);
    TEST_ASSERT_EQUAL_INT(0, decodePacket(buffer, size, &decoded));

    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, decoded.type);
    TEST_ASSERT_EQUAL_UINT32(5, decoded.seq);
    TEST_ASSERT_EQUAL_UINT16(sizeof(data), decoded.length);
    TEST_ASSERT_EQUAL_MEMORY(data, decoded.payload, sizeof(data));
}

void test_decodePacket_validAckPacket(void)
{
    Packet original;
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&original, 10));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL_UINT32(HEADER_SIZE, size);
    TEST_ASSERT_EQUAL_INT(0, decodePacket(buffer, size, &decoded));

    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, decoded.type);
    TEST_ASSERT_EQUAL_UINT32(10, decoded.seq);
    TEST_ASSERT_EQUAL_UINT16(0, decoded.length);
}

void test_decodePacket_validFinPacket(void)
{
    Packet original;
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&original, 12));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL_UINT32(HEADER_SIZE, size);
    TEST_ASSERT_EQUAL_INT(0, decodePacket(buffer, size, &decoded));

    TEST_ASSERT_EQUAL_UINT8(PACKET_FIN, decoded.type);
    TEST_ASSERT_EQUAL_UINT32(12, decoded.seq);
    TEST_ASSERT_EQUAL_UINT16(0, decoded.length);
}

void test_decodePacket_emptyDataPacket(void)
{
    Packet original;
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&original, 0, NULL, 0));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL_UINT32(HEADER_SIZE, size);
    TEST_ASSERT_EQUAL_INT(0, decodePacket(buffer, size, &decoded));

    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, decoded.type);
    TEST_ASSERT_EQUAL_UINT32(0, decoded.seq);
    TEST_ASSERT_EQUAL_UINT16(0, decoded.length);
}

void test_decodePacket_maxPayload(void)
{
    Packet original;
    Packet decoded;
    uint8_t data[MAX_PAYLOAD];
    uint8_t buffer[MAX_PACKET_SIZE];

    memset(data, 0xAB, sizeof(data));

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&original, 7, data, MAX_PAYLOAD));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL_UINT32(MAX_PACKET_SIZE, size);
    TEST_ASSERT_EQUAL_INT(0, decodePacket(buffer, size, &decoded));

    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, decoded.type);
    TEST_ASSERT_EQUAL_UINT32(7, decoded.seq);
    TEST_ASSERT_EQUAL_UINT16(MAX_PAYLOAD, decoded.length);
    TEST_ASSERT_EQUAL_MEMORY(data, decoded.payload, MAX_PAYLOAD);
}

void test_decodePacket_nullBuffer(void)
{
    Packet decoded;

    TEST_ASSERT_NOT_EQUAL(
        0, decodePacket(NULL, HEADER_SIZE, &decoded));
}

void test_decodePacket_nullPacket(void)
{
    Packet original;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&original, 1));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    TEST_ASSERT_NOT_EQUAL(0, decodePacket(buffer, size, NULL));
}

void test_decodePacket_tooShort(void)
{
    Packet decoded;
    uint8_t buffer[HEADER_SIZE] = {0};

    TEST_ASSERT_NOT_EQUAL(
        0, decodePacket(buffer, HEADER_SIZE - 1, &decoded));
}

void test_decodePacket_tooLarge(void)
{
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE + 1] = {0};

    TEST_ASSERT_NOT_EQUAL(
        0, decodePacket(buffer, sizeof(buffer), &decoded));
}

void test_decodePacket_invalidType(void)
{
    Packet original;
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&original, 1));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    buffer[0] = 99;

    TEST_ASSERT_NOT_EQUAL(0, decodePacket(buffer, size, &decoded));
}

void test_decodePacket_nonzeroReservedByte(void)
{
    Packet original;
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&original, 1));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    buffer[1] = 1;

    TEST_ASSERT_NOT_EQUAL(0, decodePacket(buffer, size, &decoded));
}

void test_decodePacket_lengthMismatch(void)
{
    Packet original;
    Packet decoded;
    uint8_t data[] = {0x11, 0x22, 0x33};
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&original, 1, data, sizeof(data)));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    /* Datagram is truncated by one byte. */
    TEST_ASSERT_NOT_EQUAL(0, decodePacket(buffer, size - 1, &decoded));
}

void test_decodePacket_corruptedPayload(void)
{
    Packet original;
    Packet decoded;
    uint8_t data[] = {0xAA, 0xBB, 0xCC};
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&original, 2, data, sizeof(data)));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    /* Change payload without updating the checksum. */
    buffer[HEADER_SIZE] ^= 0x01;

    TEST_ASSERT_NOT_EQUAL(0, decodePacket(buffer, size, &decoded));
}

void test_decodePacket_corruptedSequenceNumber(void)
{
    Packet original;
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&original, 5));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    /* Change sequence number without updating checksum. */
    buffer[7] ^= 0x01;

    TEST_ASSERT_NOT_EQUAL(0, decodePacket(buffer, size, &decoded));
}

void test_decodePacket_ackWithPayload(void)
{
    Packet decoded;
    uint8_t buffer[HEADER_SIZE + 1] = {0};

    buffer[0] = PACKET_ACK;
    buffer[8] = 0;
    buffer[9] = 1;
    buffer[HEADER_SIZE] = 0xAA;

    TEST_ASSERT_NOT_EQUAL(
        0, decodePacket(buffer, sizeof(buffer), &decoded));
}

void test_decodePacket_finWithPayload(void)
{
    Packet decoded;
    uint8_t buffer[HEADER_SIZE + 1] = {0};

    buffer[0] = PACKET_FIN;
    buffer[8] = 0;
    buffer[9] = 1;
    buffer[HEADER_SIZE] = 0xAA;

    TEST_ASSERT_NOT_EQUAL(
        0, decodePacket(buffer, sizeof(buffer), &decoded));
}

// test senderInit

void test_senderInit_validArguments(void)
{
    SenderState sender;

    int result = senderInit(&sender, 8, 250);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT32(0, sender.base);
    TEST_ASSERT_EQUAL_UINT32(0, sender.nextseqnum);
    TEST_ASSERT_EQUAL_UINT32(8, sender.window);
    TEST_ASSERT_EQUAL_UINT32(250, sender.timeout_ms);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
    TEST_ASSERT_EQUAL_UINT64(0, sender.timer_start);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_sent);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_acked);
}

void test_senderInit_minimumWindow(void)
{
    SenderState sender;

    int result = senderInit(&sender, 1, 250);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT32(1, sender.window);
}

void test_senderInit_maximumWindow(void)
{
    SenderState sender;

    int result = senderInit(&sender, MAX_WINDOW, 250);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT32(MAX_WINDOW, sender.window);
}

void test_senderInit_nullPointer(void)
{
    int result = senderInit(NULL, 8, 250);

    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_senderInit_zeroWindow(void)
{
    SenderState sender;

    int result = senderInit(&sender, 0, 250);

    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_senderInit_windowTooLarge(void)
{
    SenderState sender;

    int result = senderInit(&sender, MAX_WINDOW + 1, 250);

    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_senderInit_zeroTimeout(void)
{
    SenderState sender;

    int result = senderInit(&sender, 8, 0);

    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_senderInit_resetsState(void)
{
    SenderState sender;

    /* Fill the state with nonzero values first. */
    memset(&sender, 0xFF, sizeof(sender));

    int result = senderInit(&sender, 4, 500);

    TEST_ASSERT_EQUAL_INT(0, result);

    TEST_ASSERT_EQUAL_UINT32(0, sender.base);
    TEST_ASSERT_EQUAL_UINT32(0, sender.nextseqnum);
    TEST_ASSERT_EQUAL_UINT32(4, sender.window);
    TEST_ASSERT_EQUAL_UINT32(500, sender.timeout_ms);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
    TEST_ASSERT_EQUAL_UINT64(0, sender.timer_start);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_sent);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_acked);

    /* Verify the stored packet slots were cleared. */
    for (size_t i = 0; i < MAX_WINDOW; i++) {
        TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, sender.packets[i].type);
        TEST_ASSERT_EQUAL_UINT32(0, sender.packets[i].seq);
        TEST_ASSERT_EQUAL_UINT16(0, sender.packets[i].length);
    }

    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, sender.fin.type);
    TEST_ASSERT_EQUAL_UINT32(0, sender.fin.seq);
    TEST_ASSERT_EQUAL_UINT16(0, sender.fin.length);
}


// test senderSendData

/* Successfully sends the first DATA packet. */
void test_senderSendData_firstPacket(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 'A', 'B', 'C' };
    uint64_t now_ms = 1000;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), now_ms, &packet));

    TEST_ASSERT_EQUAL_INT(PACKET_DATA, packet.type);
    TEST_ASSERT_EQUAL_UINT32(0, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(sizeof(data), packet.length);
    TEST_ASSERT_EQUAL_MEMORY(data, packet.payload, sizeof(data));

    TEST_ASSERT_EQUAL_UINT32(1, sender.nextseqnum);
    TEST_ASSERT_EQUAL_UINT32(0, sender.base);

    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(now_ms, sender.timer_start);
}

/* Stores a copy of the packet for possible retransmission. */
void test_senderSendData_storesPacket(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 10, 20, 30, 40 };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 500, &packet));

    uint32_t index = packet.seq % MAX_WINDOW;

    TEST_ASSERT_EQUAL_INT(PACKET_DATA, sender.packets[index].type);
    TEST_ASSERT_EQUAL_UINT32(packet.seq, sender.packets[index].seq);
    TEST_ASSERT_EQUAL_UINT16(packet.length, sender.packets[index].length);
    TEST_ASSERT_EQUAL_MEMORY(
        packet.payload, sender.packets[index].payload, packet.length);
}

/* Advances the sequence number for each successfully created packet. */
void test_senderSendData_multiplePackets(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 'X', 'Y' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));
    TEST_ASSERT_EQUAL_UINT32(0, packet.seq);

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 110, &packet));
    TEST_ASSERT_EQUAL_UINT32(1, packet.seq);

    TEST_ASSERT_EQUAL_UINT32(2, sender.nextseqnum);

    /* Sending the second packet does not restart the timer. */
    TEST_ASSERT_EQUAL_UINT64(100, sender.timer_start);
}

/* Rejects a null sender. */
void test_senderSendData_nullSender(void)
{
    Packet packet;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(NULL, data, sizeof(data), 100, &packet));
}

/* Rejects a null payload pointer. */
void test_senderSendData_nullData(void)
{
    SenderState sender;
    Packet packet;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(&sender, NULL, 1, 100, &packet));
}

/* Rejects a null output packet pointer. */
void test_senderSendData_nullPacket(void)
{
    SenderState sender;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(&sender, data, sizeof(data), 100, NULL));
}

/* Rejects an empty DATA payload. */
void test_senderSendData_zeroLength(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(&sender, data, 0, 100, &packet));
}

/* Rejects payloads larger than MAX_PAYLOAD. */
void test_senderSendData_payloadTooLarge(void)
{
    SenderState sender;
    Packet packet;
    uint8_t data[MAX_PAYLOAD + 1] = { 0 };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(
            &sender, data, MAX_PAYLOAD + 1, 100, &packet));
}

/* Rejects sending when the window is full. */
void test_senderSendData_fullWindow(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 2, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));
    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));

    TEST_ASSERT_EQUAL_UINT32(2, sender.nextseqnum);

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));

    /* A failed send must not advance the sequence number. */
    TEST_ASSERT_EQUAL_UINT32(2, sender.nextseqnum);
}

/* Rejects sending after FIN has been sent. */
void test_senderSendData_afterFinSent(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));
    sender.fin_sent = 1;

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));
    TEST_ASSERT_EQUAL_UINT32(0, sender.nextseqnum);
}

/* Rejects sending after FIN has been acknowledged. */
void test_senderSendData_afterFinAcked(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));
    sender.fin_acked = 1;

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));
    TEST_ASSERT_EQUAL_UINT32(0, sender.nextseqnum);
}

//test senderHandleAck

/* Advances base when a valid cumulative ACK is received. */
void test_senderHandleAck_advancesBase(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 2;
    sender.nextseqnum = 6;
    sender.timer_running = 1;
    sender.timer_start = 100;
    sender.consecutive_timeouts = 2;

    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 4, 200));

    TEST_ASSERT_EQUAL_UINT32(4, sender.base);
    TEST_ASSERT_EQUAL_UINT32(6, sender.nextseqnum);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(200, sender.timer_start);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
}

/* Ignores a duplicate ACK without changing the sender state. */
void test_senderHandleAck_duplicateAck(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 2;
    sender.nextseqnum = 5;
    sender.timer_running = 1;
    sender.timer_start = 100;
    sender.consecutive_timeouts = 1;

    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 2, 200));

    TEST_ASSERT_EQUAL_UINT32(2, sender.base);
    TEST_ASSERT_EQUAL_UINT32(5, sender.nextseqnum);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(100, sender.timer_start);
    TEST_ASSERT_EQUAL_UINT32(1, sender.consecutive_timeouts);
}

/* Ignores an ACK older than base. */
void test_senderHandleAck_staleAck(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 3;
    sender.nextseqnum = 6;
    sender.timer_running = 1;
    sender.timer_start = 100;

    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 1, 200));

    TEST_ASSERT_EQUAL_UINT32(3, sender.base);
    TEST_ASSERT_EQUAL_UINT32(6, sender.nextseqnum);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(100, sender.timer_start);
}

/* Rejects an ACK beyond the next sequence number sent. */
void test_senderHandleAck_ackBeyondNext(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 2;
    sender.nextseqnum = 5;
    sender.timer_running = 1;
    sender.timer_start = 100;

    TEST_ASSERT_NOT_EQUAL(0, senderHandleAck(&sender, 6, 200));

    TEST_ASSERT_EQUAL_UINT32(2, sender.base);
    TEST_ASSERT_EQUAL_UINT32(5, sender.nextseqnum);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(100, sender.timer_start);
}

/* Stops the timer when all outstanding DATA packets are acknowledged. */
void test_senderHandleAck_allDataAcknowledged(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 2;
    sender.nextseqnum = 5;
    sender.timer_running = 1;
    sender.timer_start = 100;
    sender.consecutive_timeouts = 3;

    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 5, 200));

    TEST_ASSERT_EQUAL_UINT32(5, sender.base);
    TEST_ASSERT_EQUAL_UINT32(5, sender.nextseqnum);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(0, sender.timer_start);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
}

/* Handles an ACK when no DATA packets are outstanding. */
void test_senderHandleAck_noOutstandingData(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 3;
    sender.nextseqnum = 3;
    sender.timer_running = 0;

    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 3, 200));

    TEST_ASSERT_EQUAL_UINT32(3, sender.base);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
}

/* Rejects a null sender pointer. */
void test_senderHandleAck_nullSender(void)
{
    TEST_ASSERT_NOT_EQUAL(0, senderHandleAck(NULL, 1, 200));
}

/* An advancing ACK resets the consecutive timeout count. */
void test_senderHandleAck_resetsTimeoutCount(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 0;
    sender.nextseqnum = 3;
    sender.timer_running = 1;
    sender.consecutive_timeouts = 5;

    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 1, 300));

    TEST_ASSERT_EQUAL_UINT32(1, sender.base);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
}

// tests for senderHandleTimeout

/* Retransmits every outstanding packet in sequence order. */
void test_senderHandleTimeout_retransmitsAllOutstanding(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 0;
    const uint8_t data[] = { 'A', 'B', 'C' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));
    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 110, &packet));
    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 120, &packet));

    /* The timer began with the first outstanding packet at time 100. */
    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 350, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(3, count);

    TEST_ASSERT_EQUAL_UINT32(0, retransmissions[0].seq);
    TEST_ASSERT_EQUAL_UINT32(1, retransmissions[1].seq);
    TEST_ASSERT_EQUAL_UINT32(2, retransmissions[2].seq);

    for (size_t i = 0; i < count; i++) {
        TEST_ASSERT_EQUAL_INT(PACKET_DATA, retransmissions[i].type);
        TEST_ASSERT_EQUAL_UINT16(sizeof(data), retransmissions[i].length);
        TEST_ASSERT_EQUAL_MEMORY(
            data, retransmissions[i].payload, sizeof(data));
    }
}

/* Retransmits only packets that have not been acknowledged. */
void test_senderHandleTimeout_retransmitsUnacknowledgedOnly(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 0;
    const uint8_t data[] = { 'X' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));
    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 110, &packet));
    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 120, &packet));

    /* ACK 2 acknowledges packets 0 and 1. */
    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 2, 150));

    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 400, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(1, count);
    TEST_ASSERT_EQUAL_UINT32(2, retransmissions[0].seq);
}

/* Restarts the timer after retransmission. */
void test_senderHandleTimeout_restartsTimer(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 0;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));

    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 350, retransmissions, &count));

    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(350, sender.timer_start);
}

/* Increments the consecutive timeout counter. */
void test_senderHandleTimeout_incrementsTimeoutCount(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 0;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));

    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);

    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 350, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(1, sender.consecutive_timeouts);

    /* A second timeout occurs 250 ms after the restarted timer. */
    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 600, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(2, sender.consecutive_timeouts);
}

/* Rejects a timeout that occurs before the timer expires. */
void test_senderHandleTimeout_timerNotExpired(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 99;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));

    TEST_ASSERT_NOT_EQUAL(
        0, senderHandleTimeout(&sender, 349, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(0, count);
    TEST_ASSERT_EQUAL_UINT64(100, sender.timer_start);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
}

/* Rejects timeout handling when the timer is stopped. */
void test_senderHandleTimeout_timerNotRunning(void)
{
    SenderState sender;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 99;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderHandleTimeout(&sender, 1000, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(0, count);
}

/* Rejects a null sender. */
void test_senderHandleTimeout_nullSender(void)
{
    Packet retransmissions[MAX_WINDOW];
    size_t count = 0;

    TEST_ASSERT_NOT_EQUAL(
        0, senderHandleTimeout(NULL, 1000, retransmissions, &count));
}

/* Rejects a null retransmission array. */
void test_senderHandleTimeout_nullPackets(void)
{
    SenderState sender;
    size_t count = 0;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderHandleTimeout(&sender, 1000, NULL, &count));
}

/* Rejects a null count pointer. */
void test_senderHandleTimeout_nullCount(void)
{
    SenderState sender;
    Packet retransmissions[MAX_WINDOW];

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderHandleTimeout(&sender, 1000, retransmissions, NULL));
}

/* Rejects a timestamp earlier than the timer start. */
void test_senderHandleTimeout_timeWentBackward(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 99;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 500, &packet));

    TEST_ASSERT_NOT_EQUAL(
        0, senderHandleTimeout(&sender, 400, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(0, count);
    TEST_ASSERT_EQUAL_UINT64(500, sender.timer_start);
}

/* Handles the exact timer expiration boundary. */
void test_senderHandleTimeout_exactExpiration(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 0;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));

    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 350, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(1, count);
    TEST_ASSERT_EQUAL_UINT32(0, retransmissions[0].seq);
}

/* Clears the count and stops the timer when no packets are outstanding. */
void test_senderHandleTimeout_noOutstandingPackets(void)
{
    SenderState sender;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 99;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    sender.base = 2;
    sender.nextseqnum = 2;
    sender.timer_running = 1;
    sender.timer_start = 100;

    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 350, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(0, count);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(0, sender.timer_start);
}

void test_senderHandleTimeout_exceedsConfiguredWindow(void)
{
    SenderState sender;
    Packet packets[MAX_WINDOW];
    size_t count = 0;

    /* Initialize a valid sender state. */
    TEST_ASSERT_EQUAL_INT(
        0, senderInit(&sender, 4, 100)
    );

    /* Set up an expired timer. */
    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 100;

    /* Outstanding packet count = 5, window = 4. */
    sender.base = 0;
    sender.nextseqnum = 5;
    sender.window = 4;

    TEST_ASSERT_EQUAL_INT(
        1,
        senderHandleTimeout(
            &sender, 1100, packets, &count
        )
    );

    TEST_ASSERT_EQUAL_UINT(0, count);
}

// test getTimeMs

/* Test that getTimeMs returns a nonzero timestamp. */
void test_getTimeMs_returns_nonzero(void)
{
    uint64_t now = getTimeMs();

    TEST_ASSERT_TRUE(now > 0);
}

/* Test that repeated calls do not move backward. */
void test_getTimeMs_is_monotonic(void)
{
    uint64_t first = getTimeMs();
    uint64_t second = getTimeMs();

    TEST_ASSERT_TRUE(second >= first);
}

/* Test that time advances or stays the same over a short interval. */
void test_getTimeMs_does_not_decrease_over_time(void)
{
    uint64_t first = getTimeMs();

    /* Perform work between the two calls. */
    volatile unsigned int i;
    for (i = 0; i < 100000U; i++) {
        /* Intentionally empty. */
    }

    uint64_t second = getTimeMs();

    TEST_ASSERT_TRUE(second >= first);
}

/* Test that the timestamp is expressed in milliseconds,
 * rather than seconds or nanoseconds.
 */
void test_getTimeMs_reasonable_timestamp(void)
{
    uint64_t now = getTimeMs();

    /* CLOCK_MONOTONIC normally reports elapsed time since
       an unspecified starting point, not Unix epoch time.
       Check only that the value is plausible and nonzero.
    */
    TEST_ASSERT_TRUE(now > 0);
}

// test senderTimerDue


void test_senderTimerDue_null_sender(void)
{
    uint64_t remaining = 100;

    int result = senderTimerDue(NULL, 1000, &remaining);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_UINT64(100, remaining);
}

void test_senderTimerDue_null_remaining(void)
{
    SenderState sender = {0};

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 1100, NULL);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderTimerDue_timer_not_running(void)
{
    SenderState sender = {0};
    uint64_t remaining = 100;

    sender.timer_running = 0;

    int result = senderTimerDue(&sender, 1000, &remaining);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_UINT64(0, remaining);
}

void test_senderTimerDue_full_timeout_remaining(void)
{
    SenderState sender = {0};
    uint64_t remaining = 0;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 1000, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(250, remaining);
}

void test_senderTimerDue_partial_timeout_remaining(void)
{
    SenderState sender = {0};
    uint64_t remaining = 0;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 1100, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(150, remaining);
}

void test_senderTimerDue_timer_expired(void)
{
    SenderState sender = {0};
    uint64_t remaining = 100;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 1250, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(0, remaining);
}

void test_senderTimerDue_timer_expired_long_ago(void)
{
    SenderState sender = {0};
    uint64_t remaining = 100;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 2000, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(0, remaining);
}

void test_senderTimerDue_time_before_timer_start(void)
{
    SenderState sender = {0};
    uint64_t remaining = 0;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 900, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(250, remaining);
}

void test_senderTimerDue_one_ms_before_expiration(void)
{
    SenderState sender = {0};
    uint64_t remaining = 0;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 1249, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(1, remaining);
}

void test_senderTimerDue_zero_timeout(void)
{
    SenderState sender = {0};
    uint64_t remaining = 100;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 0;

    int result = senderTimerDue(&sender, 1000, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(0, remaining);
}

// test senderSendFin

void test_senderSendFin_null_sender(void)
{
    Packet packet = {0};

    int result = senderSendFin(NULL, 1000, &packet);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderSendFin_null_packet(void)
{
    SenderState sender = {0};

    sender.base = 0;
    sender.nextseqnum = 0;

    int result = senderSendFin(&sender, 1000, NULL);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderSendFin_unacknowledged_data(void)
{
    SenderState sender = {0};
    Packet packet = {0};

    sender.base = 2;
    sender.nextseqnum = 4;

    int result = senderSendFin(&sender, 1000, &packet);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_sent);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
}

void test_senderSendFin_success(void)
{
    SenderState sender = {0};
    Packet packet = {0};

    sender.base = 3;
    sender.nextseqnum = 3;
    sender.timeout_ms = 250;

    int result = senderSendFin(&sender, 1000, &packet);

    TEST_ASSERT_EQUAL_INT(0, result);

    TEST_ASSERT_EQUAL_INT(PACKET_FIN, packet.type);
    TEST_ASSERT_EQUAL_UINT32(3, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);

    TEST_ASSERT_EQUAL_INT(PACKET_FIN, sender.fin.type);
    TEST_ASSERT_EQUAL_UINT32(3, sender.fin.seq);
    TEST_ASSERT_EQUAL_UINT16(0, sender.fin.length);

    TEST_ASSERT_EQUAL_INT(1, sender.fin_sent);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_acked);

    TEST_ASSERT_EQUAL_UINT64(1000, sender.timer_start);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
}

void test_senderSendFin_empty_file(void)
{
    SenderState sender = {0};
    Packet packet = {0};

    sender.base = 0;
    sender.nextseqnum = 0;
    sender.timeout_ms = 250;

    int result = senderSendFin(&sender, 500, &packet);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(PACKET_FIN, packet.type);
    TEST_ASSERT_EQUAL_UINT32(0, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
    TEST_ASSERT_EQUAL_INT(1, sender.fin_sent);
    TEST_ASSERT_EQUAL_UINT64(500, sender.timer_start);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
}

void test_senderSendFin_already_sent(void)
{
    SenderState sender = {0};
    Packet packet = {0};

    sender.base = 3;
    sender.nextseqnum = 3;
    sender.fin_sent = 1;

    int result = senderSendFin(&sender, 1000, &packet);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderSendFin_does_not_modify_state_on_failure(void)
{
    SenderState sender = {0};
    Packet packet = {0};

    sender.base = 1;
    sender.nextseqnum = 2;
    sender.timer_start = 500;
    sender.timer_running = 1;
    sender.fin_sent = 0;

    int result = senderSendFin(&sender, 1000, &packet);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_UINT64(500, sender.timer_start);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_sent);
}

// test senderHandleFinAck

void test_senderHandleFinAck_null_sender(void)
{
    int result = senderHandleFinAck(NULL, 1, 1000);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderHandleFinAck_fin_not_sent(void)
{
    SenderState sender = {0};

    sender.fin_sent = 0;

    int result = senderHandleFinAck(&sender, 1, 1000);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_acked);
}

void test_senderHandleFinAck_already_acknowledged(void)
{
    SenderState sender = {0};

    sender.fin_sent = 1;
    sender.fin_acked = 1;
    sender.fin.seq = 5;
    sender.timer_running = 1;
    sender.timer_start = 1000;

    int result = senderHandleFinAck(&sender, 6, 1100);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(1, sender.fin_acked);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(1000, sender.timer_start);
}

void test_senderHandleFinAck_wrong_ack_too_small(void)
{
    SenderState sender = {0};

    sender.fin_sent = 1;
    sender.fin.seq = 5;
    sender.timer_running = 1;
    sender.timer_start = 1000;

    int result = senderHandleFinAck(&sender, 5, 1100);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_acked);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
}

void test_senderHandleFinAck_wrong_ack_too_large(void)
{
    SenderState sender = {0};

    sender.fin_sent = 1;
    sender.fin.seq = 5;
    sender.timer_running = 1;
    sender.timer_start = 1000;

    int result = senderHandleFinAck(&sender, 7, 1100);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_acked);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
}

void test_senderHandleFinAck_success(void)
{
    SenderState sender = {0};

    sender.fin_sent = 1;
    sender.fin_acked = 0;
    sender.fin.seq = 5;
    sender.timer_running = 1;
    sender.timer_start = 1000;

    int result = senderHandleFinAck(&sender, 6, 1100);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, sender.fin_acked);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(0, sender.timer_start);
}

void test_senderHandleFinAck_empty_file(void)
{
    SenderState sender = {0};

    sender.fin_sent = 1;
    sender.fin.seq = 0;
    sender.timer_running = 1;
    sender.timer_start = 500;

    int result = senderHandleFinAck(&sender, 1, 600);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, sender.fin_acked);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
}

void test_senderHandleFinAck_timer_already_stopped(void)
{
    SenderState sender = {0};

    sender.fin_sent = 1;
    sender.fin.seq = 3;
    sender.timer_running = 0;
    sender.timer_start = 0;

    int result = senderHandleFinAck(&sender, 4, 2000);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, sender.fin_acked);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
}

//test senderFinished

void test_senderFinished_null_sender(void)
{
    int result = senderFinished(NULL);

    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_senderFinished_no_packets_sent(void)
{
    SenderState sender = {0};

    sender.base = 0;
    sender.nextseqnum = 0;
    sender.fin_sent = 0;
    sender.fin_acked = 0;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_senderFinished_data_not_fully_acknowledged(void)
{
    SenderState sender = {0};

    sender.base = 2;
    sender.nextseqnum = 5;
    sender.fin_sent = 1;
    sender.fin_acked = 1;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_senderFinished_fin_not_sent(void)
{
    SenderState sender = {0};

    sender.base = 5;
    sender.nextseqnum = 5;
    sender.fin_sent = 0;
    sender.fin_acked = 0;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_senderFinished_fin_not_acknowledged(void)
{
    SenderState sender = {0};

    sender.base = 5;
    sender.nextseqnum = 5;
    sender.fin_sent = 1;
    sender.fin_acked = 0;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_senderFinished_success(void)
{
    SenderState sender = {0};

    sender.base = 5;
    sender.nextseqnum = 5;
    sender.fin_sent = 1;
    sender.fin_acked = 1;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderFinished_empty_file_complete(void)
{
    SenderState sender = {0};

    sender.base = 0;
    sender.nextseqnum = 0;
    sender.fin_sent = 1;
    sender.fin_acked = 1;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderFinished_data_acknowledged_but_fin_not_sent(void)
{
    SenderState sender = {0};

    sender.base = 10;
    sender.nextseqnum = 10;
    sender.fin_sent = 0;
    sender.fin_acked = 0;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(0, result);
}

// test receiverInit

void test_receiverInit_null_receiver(void)
{
    int result = receiverInit(NULL);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_receiverInit_success(void)
{
    ReceiverState receiver = {0};

    /* Set non-default values to verify initialization. */
    receiver.expectedseqnum = 10;
    receiver.finished = 1;
    receiver.last_activity = 12345;

    int result = receiverInit(&receiver);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT32(0, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_INT(0, receiver.finished);
    TEST_ASSERT_TRUE(receiver.last_activity > 0);
}

void test_receiverInit_resets_existing_state(void)
{
    ReceiverState receiver = {0};

    receiver.expectedseqnum = 25;
    receiver.finished = 1;
    receiver.last_activity = 99999;

    int result = receiverInit(&receiver);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT32(0, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_INT(0, receiver.finished);
}

void test_receiverInit_sets_activity_timestamp(void)
{
    ReceiverState receiver = {0};

    uint64_t before = getTimeMs();
    int result = receiverInit(&receiver);
    uint64_t after = getTimeMs();

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_TRUE(receiver.last_activity >= before);
    TEST_ASSERT_TRUE(receiver.last_activity <= after);
}


// test receiverHandleData

/* Accept the expected DATA packet and advance the expected sequence number. */
void test_receiverHandleData_expectedPacket(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {10, 20, 30};
    uint64_t old_activity = 0;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 0;
    receiver.finished = 0;
    receiver.last_activity = old_activity;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, data, sizeof(data)));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(1, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, ack.type);
    TEST_ASSERT_EQUAL_UINT32(1, ack.seq);
    TEST_ASSERT_EQUAL_UINT16(0, ack.length);
    TEST_ASSERT_TRUE(receiver.last_activity != old_activity);
}

/* An in-order packet advances the receiver and returns a cumulative ACK. */
void test_receiverHandleData_multipleInOrderPackets(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {42};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    for (uint32_t seq = 0; seq < 3; seq++) {
        TEST_ASSERT_EQUAL_INT(
            0, makeDataPacket(&packet, seq, data, sizeof(data)));

        TEST_ASSERT_EQUAL_INT(
            0, receiverHandleData(&receiver, &packet, 1000 + seq, &ack));

        TEST_ASSERT_EQUAL_UINT32(seq + 1, receiver.expectedseqnum);
        TEST_ASSERT_EQUAL_UINT32(seq + 1, ack.seq);
        TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, ack.type);
    }
}

/* A duplicate DATA packet must not advance expectedseqnum. */
void test_receiverHandleData_duplicatePacket(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {1, 2};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 1;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, data, sizeof(data)));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(1, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(1, ack.seq);
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, ack.type);
}

/* An out-of-order packet must be discarded logically and ACK the expected seq. */
void test_receiverHandleData_outOfOrderPacket(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {5, 6, 7};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 2;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 4, data, sizeof(data)));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(2, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(2, ack.seq);
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, ack.type);
}

/* A future packet must not cause the receiver to skip a missing sequence. */
void test_receiverHandleData_missingPacket(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {9};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 2;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 3, data, sizeof(data)));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(2, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(2, ack.seq);
}

/* A zero-length DATA packet is valid if its sequence number is expected. */
void test_receiverHandleData_zeroLengthPayload(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, NULL, 0));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(1, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(1, ack.seq);
    TEST_ASSERT_EQUAL_UINT16(0, ack.length);
}

/* A maximum-sized payload must be accepted. */
void test_receiverHandleData_maxPayload(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[MAX_PAYLOAD];

    memset(data, 0xAB, sizeof(data));

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, data, MAX_PAYLOAD));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(1, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(1, ack.seq);
}

/* A NULL receiver must return an error. */
void test_receiverHandleData_nullReceiver(void)
{
    Packet packet;
    Packet ack;
    uint8_t data[] = {1};

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, data, sizeof(data)));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleData(NULL, &packet, 1000, &ack));
}

/* A NULL input packet must return an error. */
void test_receiverHandleData_nullPacket(void)
{
    ReceiverState receiver;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleData(&receiver, NULL, 1000, &ack));
}

/* A NULL ACK output pointer must return an error. */
void test_receiverHandleData_nullAck(void)
{
    ReceiverState receiver;
    Packet packet;
    uint8_t data[] = {1};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, data, sizeof(data)));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleData(&receiver, &packet, 1000, NULL));
}

/* Non-DATA packet types must be rejected. */
void test_receiverHandleData_nonDataPacket(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&packet, 0));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(0, receiver.expectedseqnum);
}

/* Payload lengths above MAX_PAYLOAD must be rejected. */
void test_receiverHandleData_oversizedPayload(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    memset(&packet, 0, sizeof(packet));
    packet.type = PACKET_DATA;
    packet.seq = 0;
    packet.length = MAX_PAYLOAD + 1;

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(0, receiver.expectedseqnum);
}

/* A finished receiver must reject additional DATA packets. */
void test_receiverHandleData_receiverFinished(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {1};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.finished = 1;
    receiver.expectedseqnum = 3;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 3, data, sizeof(data)));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(3, receiver.expectedseqnum);
}

/* Repeated out-of-order packets should generate the same cumulative ACK. */
void test_receiverHandleData_repeatedOutOfOrderPackets(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {8};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 2;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 5, data, sizeof(data)));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));
    TEST_ASSERT_EQUAL_UINT32(2, ack.seq);

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1001, &ack));
    TEST_ASSERT_EQUAL_UINT32(2, ack.seq);

    TEST_ASSERT_EQUAL_UINT32(2, receiver.expectedseqnum);
}


// test receiverHandleFin

void test_receiverHandleFin_nullReceiver(void)
{
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 0));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleFin(NULL, &packet, 1000, &ack));
}

void test_receiverHandleFin_nullPacket(void)
{
    ReceiverState receiver;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleFin(&receiver, NULL, 1000, &ack));
}

void test_receiverHandleFin_nullAck(void)
{
    ReceiverState receiver;
    Packet packet;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 0));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleFin(&receiver, &packet, 1000, NULL));
}

void test_receiverHandleFin_nonFinPacket(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    TEST_ASSERT_EQUAL_INT(0, makeDataPacket(&packet, 0, NULL, 0));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(0, receiver.expectedseqnum);
    TEST_ASSERT_FALSE(receiver.finished);
}

void test_receiverHandleFin_nonzeroPayload(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 0));

    /* Simulate a malformed FIN packet. */
    packet.length = 1;

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(0, receiver.expectedseqnum);
    TEST_ASSERT_FALSE(receiver.finished);
}

void test_receiverHandleFin_expectedSequence(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.expectedseqnum = 3;

    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 3));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(4, receiver.expectedseqnum);
    TEST_ASSERT_TRUE(receiver.finished);
    TEST_ASSERT_EQUAL_UINT64(1000, receiver.last_activity);

    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, ack.type);
    TEST_ASSERT_EQUAL_UINT32(4, ack.seq);
    TEST_ASSERT_EQUAL_UINT16(0, ack.length);
}

void test_receiverHandleFin_emptyFile(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 0));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 500, &ack));

    TEST_ASSERT_EQUAL_UINT32(1, receiver.expectedseqnum);
    TEST_ASSERT_TRUE(receiver.finished);
    TEST_ASSERT_EQUAL_UINT32(1, ack.seq);
}

void test_receiverHandleFin_outOfOrderSequence(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.expectedseqnum = 2;

    /* Receiver expects DATA sequence 2, but FIN claims sequence 4. */
    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 4));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(2, receiver.expectedseqnum);
    TEST_ASSERT_FALSE(receiver.finished);

    /* ACK repeats the current expected sequence number. */
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, ack.type);
    TEST_ASSERT_EQUAL_UINT32(2, ack.seq);
}

void test_receiverHandleFin_earlyFin(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.expectedseqnum = 3;

    /* FIN sequence 2 is older than the receiver's expectation. */
    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 2));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(3, receiver.expectedseqnum);
    TEST_ASSERT_FALSE(receiver.finished);
    TEST_ASSERT_EQUAL_UINT32(3, ack.seq);
}

void test_receiverHandleFin_duplicateFin(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.expectedseqnum = 2;

    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 2));

    /* First FIN completes the transfer. */
    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_TRUE(receiver.finished);
    TEST_ASSERT_EQUAL_UINT32(3, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(3, ack.seq);

    /* A repeated FIN must receive the same final cumulative ACK. */
    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 1100, &ack));

    TEST_ASSERT_TRUE(receiver.finished);
    TEST_ASSERT_EQUAL_UINT32(3, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(3, ack.seq);
    TEST_ASSERT_EQUAL_UINT64(1100, receiver.last_activity);
}

void test_receiverHandleFin_unexpectedFinAfterCompletion(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 4;
    receiver.finished = 1;

    /* The accepted FIN would have had sequence number 3. */
    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 5));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(4, receiver.expectedseqnum);
    TEST_ASSERT_TRUE(receiver.finished);
}

void test_receiverHandleFin_activityTimestamp(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 123;

    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 0));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 9876, &ack));

    TEST_ASSERT_EQUAL_UINT64(9876, receiver.last_activity);
}


// test receiverTimerExpired

void test_receiverTimerExpired_nullReceiver(void)
{
    TEST_ASSERT_NOT_EQUAL(0, receiverTimerExpired(NULL, 1000));
}

void test_receiverTimerExpired_noElapsedTime(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 1000;

    TEST_ASSERT_EQUAL_INT(0, receiverTimerExpired(&receiver, 1000));
}

void test_receiverTimerExpired_lessThanTimeout(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 1000;

    /* 29,999 ms have elapsed. */
    TEST_ASSERT_EQUAL_INT(0, receiverTimerExpired(&receiver, 30999));
}

void test_receiverTimerExpired_exactlyAtTimeout(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 1000;

    /* Exactly 30,000 ms have elapsed. */
    TEST_ASSERT_EQUAL_INT(1, receiverTimerExpired(&receiver, 31000));
}

void test_receiverTimerExpired_pastTimeout(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 1000;

    /* 30,001 ms have elapsed. */
    TEST_ASSERT_EQUAL_INT(1, receiverTimerExpired(&receiver, 31001));
}

void test_receiverTimerExpired_timeBeforeLastActivity(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 5000;

    /* A timestamp earlier than last_activity must not cause underflow. */
    TEST_ASSERT_EQUAL_INT(0, receiverTimerExpired(&receiver, 4000));
}

void test_receiverTimerExpired_zeroLastActivity(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 0;

    TEST_ASSERT_EQUAL_INT(0, receiverTimerExpired(&receiver, 29999));
    TEST_ASSERT_EQUAL_INT(1, receiverTimerExpired(&receiver, 30000));
}

void test_receiverTimerExpired_largeTimestamps(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.last_activity = UINT64_MAX - 40000;

    /* 29,999 ms have elapsed. */
    TEST_ASSERT_EQUAL_INT(
        0, receiverTimerExpired(&receiver, UINT64_MAX - 10001));

    /* Exactly 40,000 ms have elapsed, exceeding the timeout. */
    TEST_ASSERT_EQUAL_INT(
        1, receiverTimerExpired(&receiver, UINT64_MAX));
}

void test_receiverTimerExpired_doesNotModifyState(void)
{
    ReceiverState receiver;
    uint32_t expected;
    int finished;
    uint64_t last_activity;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 5;
    receiver.finished = 0;
    receiver.last_activity = 1000;

    expected = receiver.expectedseqnum;
    finished = receiver.finished;
    last_activity = receiver.last_activity;

    (void)receiverTimerExpired(&receiver, 31000);

    TEST_ASSERT_EQUAL_UINT32(expected, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_INT(finished, receiver.finished);
    TEST_ASSERT_EQUAL_UINT64(last_activity, receiver.last_activity);
}

// test resolveAddr

void test_resolveAddr_validIPv4(void)
{
    struct addrinfo *results = NULL;

    TEST_ASSERT_EQUAL_INT(
        0, resolveAddr("127.0.0.1", "4250", &results));

    TEST_ASSERT_NOT_NULL(results);
    TEST_ASSERT_EQUAL_INT(AF_INET, results->ai_family);
    TEST_ASSERT_EQUAL_INT(SOCK_DGRAM, results->ai_socktype);
    TEST_ASSERT_EQUAL_INT(IPPROTO_UDP, results->ai_protocol);

    freeaddrinfo(results);
}

void test_resolveAddr_validHostname(void)
{
    struct addrinfo *results = NULL;

    TEST_ASSERT_EQUAL_INT(
        0, resolveAddr("localhost", "4250", &results));

    TEST_ASSERT_NOT_NULL(results);

    /* AF_UNSPEC permits either IPv4 or IPv6 results. */
    TEST_ASSERT_TRUE(
        results->ai_family == AF_INET ||
        results->ai_family == AF_INET6);

    TEST_ASSERT_EQUAL_INT(SOCK_DGRAM, results->ai_socktype);

    freeaddrinfo(results);
}

void test_resolveAddr_invalidHostname(void)
{
    struct addrinfo *results = NULL;

    TEST_ASSERT_NOT_EQUAL(
        0,
        resolveAddr(
            "this-hostname-should-not-exist.invalid",
            "4250",
            &results));

    TEST_ASSERT_NULL(results);
}

void test_resolveAddr_nullHost(void)
{
    struct addrinfo *results = NULL;

    TEST_ASSERT_NOT_EQUAL(
        0, resolveAddr(NULL, "4250", &results));

    TEST_ASSERT_NULL(results);
}

void test_resolveAddr_nullPort(void)
{
    struct addrinfo *results = NULL;

    TEST_ASSERT_NOT_EQUAL(
        0, resolveAddr("127.0.0.1", NULL, &results));

    TEST_ASSERT_NULL(results);
}

void test_resolveAddr_nullResults(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0, resolveAddr("127.0.0.1", "4250", NULL));
}

void test_resolveAddr_failureClearsResults(void)
{
    /* A failed resolution must not leave a stale output pointer. */
    struct addrinfo *results = (struct addrinfo *)1;

    TEST_ASSERT_NOT_EQUAL(
        0,
        resolveAddr(
            "this-hostname-should-not-exist.invalid",
            "4250",
            &results));

    TEST_ASSERT_NULL(results);
}

void test_resolveAddr_numericService(void)
{
    struct addrinfo *results = NULL;

    TEST_ASSERT_EQUAL_INT(
        0, resolveAddr("127.0.0.1", "4250", &results));

    TEST_ASSERT_NOT_NULL(results);
    TEST_ASSERT_NOT_NULL(results->ai_addr);

    freeaddrinfo(results);
}

// test createSocket

/* Test creating a valid UDP IPv4 socket. */
void test_createSocket_validUDPIPv4(void)
{
    struct addrinfo addr = {0};
    int sockfd;

    addr.ai_family = AF_INET;
    addr.ai_socktype = SOCK_DGRAM;
    addr.ai_protocol = IPPROTO_UDP;

    sockfd = createSocket(&addr);

    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, sockfd);

    close(sockfd);
}

/* Test creating a valid UDP IPv6 socket. */
void test_createSocket_validUDPIPv6(void)
{
    struct addrinfo addr = {0};
    int sockfd;

    addr.ai_family = AF_INET6;
    addr.ai_socktype = SOCK_DGRAM;
    addr.ai_protocol = IPPROTO_UDP;

    sockfd = createSocket(&addr);

    /*
     * IPv6 may be disabled on some systems.
     * If socket creation succeeds, close the descriptor.
     */
    if (sockfd >= 0) {
        close(sockfd);
    }
}

/* Test that a NULL address is rejected. */
void test_createSocket_nullAddress(void)
{
    TEST_ASSERT_EQUAL_INT(-1, createSocket(NULL));
}

/* Test that an invalid address family fails. */
void test_createSocket_invalidFamily(void)
{
    struct addrinfo addr = {0};

    addr.ai_family = -1;
    addr.ai_socktype = SOCK_DGRAM;
    addr.ai_protocol = IPPROTO_UDP;

    TEST_ASSERT_EQUAL_INT(-1, createSocket(&addr));
}

/* Test that an invalid socket type fails. */
void test_createSocket_invalidSocketType(void)
{
    struct addrinfo addr = {0};

    addr.ai_family = AF_INET;
    addr.ai_socktype = -1;
    addr.ai_protocol = IPPROTO_UDP;

    TEST_ASSERT_EQUAL_INT(-1, createSocket(&addr));
}

/* Test that an unsupported protocol fails. */
void test_createSocket_invalidProtocol(void)
{
    struct addrinfo addr = {0};

    addr.ai_family = AF_INET;
    addr.ai_socktype = SOCK_DGRAM;
    addr.ai_protocol = -1;

    TEST_ASSERT_EQUAL_INT(-1, createSocket(&addr));
}

// test registerReceiver

/*
 * Start a relay process that receives one HELLO message
 * and replies with the specified response.
 */
static int startTestRelay(const char *response,
                          const char *expected_hello)
{
    relay_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (relay_fd < 0) {
        return -1;
    }

    relay_addr.sin_family = AF_INET;
    relay_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    relay_addr.sin_port = 0;

    if (bind(relay_fd,
             (struct sockaddr *)&relay_addr,
             sizeof(relay_addr)) < 0) {
        close(relay_fd);
        relay_fd = -1;
        return -1;
    }

    socklen_t addr_len = sizeof(relay_addr);

    if (getsockname(relay_fd,
                    (struct sockaddr *)&relay_addr,
                    &addr_len) < 0) {
        close(relay_fd);
        relay_fd = -1;
        return -1;
    }

    relay_pid = fork();

    if (relay_pid < 0) {
        close(relay_fd);
        relay_fd = -1;
        return -1;
    }

    if (relay_pid == 0) {
        /* Child process acts as the relay. */
        char buffer[128];
        struct sockaddr_storage client_addr;
        socklen_t client_len = sizeof(client_addr);

        ssize_t received = recvfrom(
            relay_fd,
            buffer,
            sizeof(buffer) - 1,
            0,
            (struct sockaddr *)&client_addr,
            &client_len);

        if (received < 0) {
            _exit(2);
        }

        buffer[received] = '\0';

        if (strcmp(buffer, expected_hello) != 0) {
            _exit(3);
        }

        ssize_t sent = sendto(
            relay_fd,
            response,
            strlen(response),
            0,
            (struct sockaddr *)&client_addr,
            client_len);

        _exit(sent == (ssize_t)strlen(response) ? 0 : 4);
    }

    return 0;
}

/* Create a client UDP socket. */
static int createTestClient(void)
{
    return socket(AF_INET, SOCK_DGRAM, 0);
}

/* Test successful registration. */
void test_registerReceiver_success(void)
{
    TEST_ASSERT_EQUAL_INT(
        0, startTestRelay("OK", "HELLO ta-2 recv"));

    int sockfd = createTestClient();
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerReceiver(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2");

    TEST_ASSERT_EQUAL_INT(0, result);

    close(sockfd);
}

/* Test that the relay's error response is rejected. */
void test_registerReceiver_relayError(void)
{
    TEST_ASSERT_EQUAL_INT(
        0, startTestRelay("ERR receiver required",
                          "HELLO ta-2 recv"));

    int sockfd = createTestClient();
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerReceiver(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2");

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test a null socket address. */
void test_registerReceiver_nullRelay(void)
{
    int sockfd = createTestClient();
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerReceiver(
        sockfd, NULL, sizeof(relay_addr), "ta-2");

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test a null session name. */
void test_registerReceiver_nullSession(void)
{
    int sockfd = createTestClient();
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerReceiver(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        NULL);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test an invalid session character. */
void test_registerReceiver_invalidSessionCharacter(void)
{
    int sockfd = createTestClient();
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerReceiver(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta_2");

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test a session name longer than 32 characters. */
void test_registerReceiver_sessionTooLong(void)
{
    int sockfd = createTestClient();
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerReceiver(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "abcdefghijklmnopqrstuvwxyz1234567");

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test an invalid socket descriptor. */
void test_registerReceiver_invalidSocket(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        registerReceiver(
            -1,
            (const struct sockaddr *)&relay_addr,
            sizeof(relay_addr),
            "ta-2"));
}

void test_registerReceiver_ipv6_address_check(void)
{
    int client_fd = -1;
    int test_relay_fd = -1;
    int wrong_fd = -1;
    pid_t child_pid = -1;

    struct sockaddr_in6 relay6 = {0};
    struct sockaddr_in6 wrong6 = {0};

    client_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    test_relay_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    wrong_fd = socket(AF_INET6, SOCK_DGRAM, 0);

    TEST_ASSERT_TRUE(client_fd >= 0);
    TEST_ASSERT_TRUE(test_relay_fd >= 0);
    TEST_ASSERT_TRUE(wrong_fd >= 0);

    relay6.sin6_family = AF_INET6;
    relay6.sin6_addr = in6addr_loopback;
    relay6.sin6_port = 0;

    wrong6.sin6_family = AF_INET6;
    wrong6.sin6_addr = in6addr_loopback;
    wrong6.sin6_port = 0;

    TEST_ASSERT_EQUAL_INT(
        0, bind(test_relay_fd, (struct sockaddr *)&relay6, sizeof(relay6)));
    TEST_ASSERT_EQUAL_INT(
        0, bind(wrong_fd, (struct sockaddr *)&wrong6, sizeof(wrong6)));

    socklen_t relay_len = sizeof(relay6);
    socklen_t wrong_len = sizeof(wrong6);

    TEST_ASSERT_EQUAL_INT(
        0, getsockname(test_relay_fd, (struct sockaddr *)&relay6, &relay_len));
    TEST_ASSERT_EQUAL_INT(
        0, getsockname(wrong_fd, (struct sockaddr *)&wrong6, &wrong_len));

    child_pid = fork();
    TEST_ASSERT_TRUE(child_pid >= 0);

    if (child_pid == 0) {
        char buffer[128];
        struct sockaddr_storage client_addr;
        socklen_t client_len = sizeof(client_addr);

        ssize_t received = recvfrom(
            test_relay_fd, buffer, sizeof(buffer) - 1, 0,
            (struct sockaddr *)&client_addr, &client_len);

        if (received < 0) {
            _exit(2);
        }

        buffer[received] = '\0';

        if (strcmp(buffer, "HELLO ta-2 recv") != 0) {
            _exit(3);
        }

        /* Send an invalid reply from a different IPv6 port. */
        if (sendto(wrong_fd, "OK", 2, 0,
                   (struct sockaddr *)&client_addr, client_len) != 2) {
            _exit(4);
        }

        /* Then send the valid reply from the expected relay address. */
        if (sendto(test_relay_fd, "OK", 2, 0,
                   (struct sockaddr *)&client_addr, client_len) != 2) {
            _exit(5);
        }

        _exit(0);
    }

    int result = registerReceiver(
        client_fd, (struct sockaddr *)&relay6, relay_len, "ta-2");

    TEST_ASSERT_EQUAL_INT(0, result);

    int status = 0;
    TEST_ASSERT_EQUAL_INT(child_pid, waitpid(child_pid, &status, 0));
    TEST_ASSERT_TRUE(WIFEXITED(status));
    TEST_ASSERT_EQUAL_INT(0, WEXITSTATUS(status));

    close(client_fd);
    close(test_relay_fd);
    close(wrong_fd);
}

void test_registerReceiver_fails_after_five_attempts(void)
{
    int client_fd = -1;
    int test_relay_fd = -1;
    pid_t child_pid = -1;

    struct sockaddr_in relay = {0};

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    test_relay_fd = socket(AF_INET, SOCK_DGRAM, 0);

    TEST_ASSERT_TRUE(client_fd >= 0);
    TEST_ASSERT_TRUE(test_relay_fd >= 0);

    relay.sin_family = AF_INET;
    relay.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    relay.sin_port = 0;

    TEST_ASSERT_EQUAL_INT(
        0, bind(test_relay_fd, (struct sockaddr *)&relay, sizeof(relay)));

    socklen_t relay_len = sizeof(relay);
    TEST_ASSERT_EQUAL_INT(
        0, getsockname(test_relay_fd, (struct sockaddr *)&relay, &relay_len));

    child_pid = fork();
    TEST_ASSERT_TRUE(child_pid >= 0);

    if (child_pid == 0) {
        char buffer[128];

        for (int i = 0; i < 5; i++) {
            struct sockaddr_storage client_addr;
            socklen_t client_len = sizeof(client_addr);

            ssize_t received = recvfrom(
                test_relay_fd, buffer, sizeof(buffer) - 1, 0,
                (struct sockaddr *)&client_addr, &client_len);

            if (received < 0) {
                _exit(2);
            }

            buffer[received] = '\0';

            if (strcmp(buffer, "HELLO ta-2 recv") != 0) {
                _exit(3);
            }

            /* Reply with an invalid response on every attempt. */
            if (sendto(test_relay_fd, "NOPE", 4, 0,
                       (struct sockaddr *)&client_addr, client_len) != 4) {
                _exit(4);
            }
        }

        _exit(0);
    }

    int result = registerReceiver(
        client_fd, (struct sockaddr *)&relay, relay_len, "ta-2");

    TEST_ASSERT_EQUAL_INT(1, result);

    int status = 0;
    TEST_ASSERT_EQUAL_INT(child_pid, waitpid(child_pid, &status, 0));
    TEST_ASSERT_TRUE(WIFEXITED(status));
    TEST_ASSERT_EQUAL_INT(0, WEXITSTATUS(status));

    close(client_fd);
    close(test_relay_fd);
}

// test registerSender

/* Test successful sender registration. */
void test_registerSender_success(void)
{
    TEST_ASSERT_EQUAL_INT(
        0,
        startTestRelay(
            "OK",
            "HELLO ta-2 send 0.1 0.05 0"));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2",
        0.1,
        0.05,
        0.0);

    TEST_ASSERT_EQUAL_INT(0, result);

    close(sockfd);
}

/* Test that the sender rejects an ERR response. */
void test_registerSender_relayError(void)
{
    TEST_ASSERT_EQUAL_INT(
        0,
        startTestRelay(
            "ERR receiver required",
            "HELLO ta-2 send 0.1 0.05 0"));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2",
        0.1,
        0.05,
        0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test a null relay address. */
void test_registerSender_nullRelay(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd, NULL, sizeof(relay_addr),
        "ta-2", 0.1, 0.05, 0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test a null session. */
void test_registerSender_nullSession(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        NULL,
        0.1, 0.05, 0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test rejection of invalid session characters. */
void test_registerSender_invalidSession(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta_2",
        0.1, 0.05, 0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test rejection of a session longer than 32 characters. */
void test_registerSender_sessionTooLong(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "abcdefghijklmnopqrstuvwxyz1234567",
        0.1, 0.05, 0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test rejection of a negative loss probability. */
void test_registerSender_negativeLoss(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2",
        -0.1, 0.05, 0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test rejection of a corruption probability greater than 0.5. */
void test_registerSender_corruptTooHigh(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2",
        0.1, 0.6, 0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test rejection of a duplication probability greater than 0.5. */
void test_registerSender_dupTooHigh(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2",
        0.1, 0.05, 0.8);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test rejection of an invalid socket descriptor. */
void test_registerSender_invalidSocket(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        registerSender(
            -1,
            (const struct sockaddr *)&relay_addr,
            sizeof(relay_addr),
            "ta-2",
            0.1, 0.05, 0.0));
}

void test_registerSender_ipv6_success(void)
{
    /* Create an IPv6 UDP relay socket. */
    relay_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(relay_fd >= 0);

    struct sockaddr_in6 relay_addr6;
    memset(&relay_addr6, 0, sizeof(relay_addr6));

    relay_addr6.sin6_family = AF_INET6;
    relay_addr6.sin6_addr = in6addr_loopback;
    relay_addr6.sin6_port = 0;

    TEST_ASSERT_EQUAL_INT(
        0,
        bind(relay_fd,
             (struct sockaddr *)&relay_addr6,
             sizeof(relay_addr6)));

    socklen_t relay_len = sizeof(relay_addr6);

    TEST_ASSERT_EQUAL_INT(
        0,
        getsockname(relay_fd,
                    (struct sockaddr *)&relay_addr6,
                    &relay_len));

    /* Fork a child process to act as the relay. */
    relay_pid = fork();
    TEST_ASSERT_TRUE(relay_pid >= 0);

    if (relay_pid == 0) {
        char buffer[128];
        struct sockaddr_in6 client_addr;
        socklen_t client_len = sizeof(client_addr);

        ssize_t received = recvfrom(
            relay_fd,
            buffer,
            sizeof(buffer) - 1,
            0,
            (struct sockaddr *)&client_addr,
            &client_len);

        if (received < 0) {
            _exit(1);
        }

        buffer[received] = '\0';

        if (strcmp(buffer, "HELLO ta-2 send 0.1 0.05 0") != 0) {
            _exit(2);
        }

        const char response[] = "OK";

        ssize_t sent = sendto(
            relay_fd,
            response,
            sizeof(response) - 1,
            0,
            (struct sockaddr *)&client_addr,
            client_len);

        _exit(sent == (ssize_t)(sizeof(response) - 1) ? 0 : 3);
    }

    /* Create an IPv6 client socket. */
    int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    /* Register the sender using an IPv6 relay address. */
    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr6,
        sizeof(relay_addr6),
        "ta-2",
        0.1,
        0.05,
        0.0);

    close(sockfd);

    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_registerSender_wrongRelayAddress(void)
{
    /* Socket representing the expected relay address. */
    relay_fd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(relay_fd >= 0);

    memset(&relay_addr, 0, sizeof(relay_addr));
    relay_addr.sin_family = AF_INET;
    relay_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    relay_addr.sin_port = 0;

    TEST_ASSERT_EQUAL_INT(
        0,
        bind(relay_fd,
             (struct sockaddr *)&relay_addr,
             sizeof(relay_addr)));

    socklen_t relay_len = sizeof(relay_addr);
    TEST_ASSERT_EQUAL_INT(
        0,
        getsockname(relay_fd,
                    (struct sockaddr *)&relay_addr,
                    &relay_len));

    /* Create a different socket to send the response. */
    int wrong_fd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(wrong_fd >= 0);

    struct sockaddr_in wrong_addr;
    memset(&wrong_addr, 0, sizeof(wrong_addr));
    wrong_addr.sin_family = AF_INET;
    wrong_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    wrong_addr.sin_port = 0;

    TEST_ASSERT_EQUAL_INT(
        0,
        bind(wrong_fd,
             (struct sockaddr *)&wrong_addr,
             sizeof(wrong_addr)));

    socklen_t wrong_len = sizeof(wrong_addr);
    TEST_ASSERT_EQUAL_INT(
        0,
        getsockname(wrong_fd,
                    (struct sockaddr *)&wrong_addr,
                    &wrong_len));

    relay_pid = fork();
    TEST_ASSERT_TRUE(relay_pid >= 0);

    if (relay_pid == 0) {
        /* Keep relay_fd open so HELLO reaches the child. */

        for (int i = 0; i < 5; i++) {
            char buffer[128];
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            ssize_t received = recvfrom(
                relay_fd,
                buffer,
                sizeof(buffer),
                0,
                (struct sockaddr *)&client_addr,
                &client_len
            );

            if (received < 0) {
                _exit(1);
            }

            /* Respond from a different port than relay_fd. */
            ssize_t sent = sendto(
                wrong_fd,
                "OK",
                2,
                0,
                (struct sockaddr *)&client_addr,
                client_len
            );

            if (sent != 2) {
                _exit(2);
            }
        }

        _exit(0);
    }

    close(wrong_fd);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2",
        0.1,
        0.05,
        0.0);

    close(sockfd);

    /* No response came from the expected relay address. */
    TEST_ASSERT_NOT_EQUAL(0, result);
}

// runSender tests

/*
 * Create a temporary file containing the supplied data.
 * Returns 0 on success and -1 on failure.
 */
static int createTempFile(const char *contents,
                          char *path,
                          size_t path_size)
{
    const char *template_path = "/tmp/runSender_test_XXXXXX";
    int fd;

    if (path == NULL || path_size < strlen(template_path) + 1) {
        return -1;
    }

    strcpy(path, template_path);

    fd = mkstemp(path);
    if (fd < 0) {
        return -1;
    }

    if (contents != NULL) {
        size_t length = strlen(contents);
        size_t written = 0;

        while (written < length) {
            ssize_t n = write(fd, contents + written,
                              length - written);

            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }

                close(fd);
                unlink(path);
                return -1;
            }

            written += (size_t)n;
        }
    }

    if (close(fd) != 0) {
        unlink(path);
        return -1;
    }

    return 0;
}

/*
 * Start a local UDP relay that rejects sender registration.
 *
 * The relay replies "ERR test rejection" to each registration
 * datagram. The listening socket is bound to an available local port.
 *
 * Returns 0 on success and -1 on failure.
 */
static int startRejectingRelay(void)
{
    int sockfd;
    struct sockaddr_in address;
    socklen_t address_len = sizeof(address);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return -1;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(0);

    if (bind(sockfd, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        close(sockfd);
        return -1;
    }

    if (getsockname(sockfd, (struct sockaddr *)&address,
                    &address_len) < 0) {
        close(sockfd);
        return -1;
    }

    if (snprintf(relay_port, sizeof(relay_port), "%u",
                 (unsigned int)ntohs(address.sin_port)) >=
        (int)sizeof(relay_port)) {
        close(sockfd);
        return -1;
    }

    relay_pid = fork();

    if (relay_pid < 0) {
        close(sockfd);
        return -1;
    }

    if (relay_pid == 0) {
        /*
         * Child process: handle registration requests.
         * A timeout prevents the child from blocking indefinitely
         * if the test exits before all retries are sent.
         */
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        (void)setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
                         &timeout, sizeof(timeout));

        for (int i = 0; i < 5; i++) {
            char request[256];
            struct sockaddr_storage client;
            socklen_t client_len = sizeof(client);

            ssize_t received =
                recvfrom(sockfd, request, sizeof(request), 0,
                         (struct sockaddr *)&client, &client_len);

            if (received < 0) {
                if (errno == EINTR) {
                    i--;
                    continue;
                }

                break;
            }

            static const char response[] = "ERR test rejection";

            (void)sendto(sockfd, response, sizeof(response) - 1,
                         0, (struct sockaddr *)&client, client_len);
        }

        close(sockfd);
        _exit(0);
    }

    /* Parent process no longer needs its copy of the socket. */
    close(sockfd);
    return 0;
}

/* ---------- Argument validation tests ---------- */

void test_runSender_nullRelay_returnsFailure(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender(NULL, "4250", "ta-2", "input.txt",
                  8, 250, 0.0, 0.0, 0.0)
    );
}

void test_runSender_nullFilename_returnsFailure(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", "4250", "ta-2", NULL,
                  8, 250, 0.0, 0.0, 0.0)
    );
}

void test_runSender_zeroWindow_returnsFailure(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", "4250", "ta-2", "input.txt",
                  0, 250, 0.0, 0.0, 0.0)
    );
}

void test_runSender_windowAboveMaximum_returnsFailure(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", "4250", "ta-2", "input.txt",
                  MAX_WINDOW + 1, 250, 0.0, 0.0, 0.0)
    );
}

void test_runSender_zeroTimeout_returnsFailure(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", "4250", "ta-2", "input.txt",
                  8, 0, 0.0, 0.0, 0.0)
    );
}

/* ---------- File handling tests ---------- */

void test_runSender_missingFile_returnsFailure(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", "4250", "ta-2",
                  "/tmp/runSender_file_that_does_not_exist",
                  8, 250, 0.0, 0.0, 0.0)
    );
}

/* ---------- Address resolution tests ---------- */

void test_runSender_invalidPort_returnsFailure(void)
{
    char filename[64];

    TEST_ASSERT_EQUAL_INT(
        0, createTempFile("test data", filename, sizeof(filename))
    );

    /*
     * The service name is deliberately invalid, so address
     * resolution should fail before relay registration.
     */
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", "invalid_port", "ta-2",
                  filename, 8, 250, 0.0, 0.0, 0.0)
    );

    unlink(filename);
}

/* ---------- Relay registration tests ---------- */

void test_runSender_relayRejectsRegistration_returnsFailure(void)
{
    char filename[64];

    TEST_ASSERT_EQUAL_INT(
        0, createTempFile("test data", filename, sizeof(filename))
    );

    TEST_ASSERT_EQUAL_INT(0, startRejectingRelay());

    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", relay_port, "ta-2",
                  filename, 8, 250, 0.0, 0.0, 0.0)
    );

    unlink(filename);
}


// test runReceiver

/*
 * Wait for a datagram on the mock relay socket.
 * Returns the received byte count, or -1 on failure/timeout.
 */
static ssize_t testRecvDatagram(uint8_t *buffer, size_t capacity,
                                struct sockaddr_in *source,
                                int timeout_ms)
{
    struct pollfd pfd;
    socklen_t source_len = sizeof(*source);

    pfd.fd = test_relay_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    int result = poll(&pfd, 1, timeout_ms);

    if (result <= 0 || !(pfd.revents & POLLIN)) {
        return -1;
    }

    return recvfrom(test_relay_fd, buffer, capacity, 0,
                    (struct sockaddr *)source, &source_len);
}

/*
 * Send an encoded packet from the mock relay to the receiver.
 */
static int testSendPacket(const Packet *packet)
{
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size;

    if (encodePacket(packet, buffer, sizeof(buffer),
                     &packet_size) != 0) {
        return -1;
    }

    ssize_t sent = sendto(
        test_relay_fd, buffer, packet_size, 0,
        (struct sockaddr *)&test_receiver_addr,
        sizeof(test_receiver_addr));

    return sent == (ssize_t)packet_size ? 0 : -1;
}

/*
 * Receive and decode an ACK from the receiver.
 */
static int testReceiveAck(uint32_t expected_seq)
{
    uint8_t buffer[MAX_PACKET_SIZE];
    struct sockaddr_in source;
    Packet ack;

    ssize_t received = testRecvDatagram(
        buffer, sizeof(buffer), &source, TEST_TIMEOUT_MS);

    if (received < 0) {
        return -1;
    }

    if (decodePacket(buffer, (size_t)received, &ack) != 0) {
        return -1;
    }

    if (ack.type != PACKET_ACK ||
        ack.seq != expected_seq ||
        ack.length != 0) {
        return -1;
    }

    return 0;
}

/*
 * Start a mock relay and a child process running runReceiver().
 * The parent process completes the receiver's registration.
 */
static int testStartReceiver(void)
{
    struct sockaddr_in relay_addr;
    socklen_t relay_len = sizeof(relay_addr);
    char port[TEST_PORT_SIZE];
    uint8_t buffer[MAX_PACKET_SIZE];
    struct sockaddr_in source;
    ssize_t received;

    test_relay_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (test_relay_fd < 0) {
        return -1;
    }

    memset(&relay_addr, 0, sizeof(relay_addr));
    relay_addr.sin_family = AF_INET;
    relay_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    relay_addr.sin_port = 0;

    if (bind(test_relay_fd, (struct sockaddr *)&relay_addr,
             sizeof(relay_addr)) < 0) {
        return -1;
    }

    if (getsockname(test_relay_fd,
                    (struct sockaddr *)&relay_addr,
                    &relay_len) < 0) {
        return -1;
    }

    if (snprintf(port, sizeof(port), "%u",
                 (unsigned int)ntohs(relay_addr.sin_port))
        >= (int)sizeof(port)) {
        return -1;
    }

    snprintf(test_output_path, sizeof(test_output_path),
             "/tmp/runReceiver-test-%ld.bin", (long)getpid());

    test_receiver_pid = fork();

    if (test_receiver_pid < 0) {
        return -1;
    }

    if (test_receiver_pid == 0) {
        /*
         * The receiver opens its own socket.
         * The inherited relay descriptor is unused by the child.
         */
        int result = runReceiver(
            "127.0.0.1", port, "ta-2", test_output_path);

        _exit(result == 0 ? 0 : 1);
    }

    /*
     * Receive the receiver's HELLO registration.
     */
    received = testRecvDatagram(
        buffer, sizeof(buffer), &source, TEST_TIMEOUT_MS);

    if (received < 0) {
        return -1;
    }

    buffer[(size_t)received < sizeof(buffer)
           ? (size_t)received : sizeof(buffer) - 1] = '\0';

    if (strcmp((char *)buffer, "HELLO ta-2 recv") != 0) {
        return -1;
    }

    test_receiver_addr = source;

    /*
     * Complete registration.
     */
    if (sendto(test_relay_fd, "OK", 2, 0,
               (struct sockaddr *)&test_receiver_addr,
               sizeof(test_receiver_addr)) != 2) {
        return -1;
    }

    return 0;
}


/*
 * Send FIN and check the final cumulative ACK.
 */
static int testSendFinAndFinish(uint32_t seq)
{
    Packet fin;

    if (makeFinPacket(&fin, seq) != 0) {
        return -1;
    }

    if (testSendPacket(&fin) != 0) {
        return -1;
    }

    /*
     * FIN advances the receiver's expected sequence number,
     * so the final ACK is seq + 1.
     */
    return testReceiveAck(seq + 1);
}

/*
 * A valid DATA packet followed by FIN should produce cumulative
 * ACKs and write the original payload to the output file.
 */
void test_runReceiver_validDataAndFin(void)
{
    const uint8_t content[] = {
        'H', 'e', 'l', 'l', 'o', '\0', 'U', 'D', 'P'
    };

    Packet data;
    FILE *file;
    uint8_t actual[sizeof(content)];
    size_t count;

    TEST_ASSERT_EQUAL_INT(0, testStartReceiver());

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&data, 0, content, sizeof(content)));

    TEST_ASSERT_EQUAL_INT(0, testSendPacket(&data));
    TEST_ASSERT_EQUAL_INT(0, testReceiveAck(1));

    TEST_ASSERT_EQUAL_INT(0, testSendFinAndFinish(1));

    /*
     * The child exits after its FIN linger period.
     */
    int status;
    TEST_ASSERT_EQUAL_INT(
        test_receiver_pid,
        waitpid(test_receiver_pid, &status, 0));

    TEST_ASSERT_TRUE(WIFEXITED(status));
    TEST_ASSERT_EQUAL_INT(0, WEXITSTATUS(status));
    test_receiver_pid = -1;

    file = fopen(test_output_path, "rb");
    TEST_ASSERT_NOT_NULL(file);

    count = fread(actual, 1, sizeof(actual), file);

    TEST_ASSERT_EQUAL_size_t(sizeof(content), count);
    TEST_ASSERT_EQUAL_MEMORY(content, actual, sizeof(content));
    TEST_ASSERT_EQUAL_INT(0, fclose(file));
}

/*
 * An empty file consists of a FIN packet with sequence number 0.
 * The receiver should create an empty output file and exit cleanly.
 */
void test_runReceiver_emptyFile(void)
{
    FILE *file;
    int status;

    TEST_ASSERT_EQUAL_INT(0, testStartReceiver());
    TEST_ASSERT_EQUAL_INT(0, testSendFinAndFinish(0));

    TEST_ASSERT_EQUAL_INT(
        test_receiver_pid,
        waitpid(test_receiver_pid, &status, 0));

    TEST_ASSERT_TRUE(WIFEXITED(status));
    TEST_ASSERT_EQUAL_INT(0, WEXITSTATUS(status));
    test_receiver_pid = -1;

    file = fopen(test_output_path, "rb");
    TEST_ASSERT_NOT_NULL(file);

    TEST_ASSERT_EQUAL_INT(EOF, fgetc(file));
    TEST_ASSERT_EQUAL_INT(0, fclose(file));
}

/*
 * A corrupted datagram must be silently discarded.
 * The following valid DATA packet should still be accepted.
 */
void test_runReceiver_discardsCorruptedPacket(void)
{
    Packet data;
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size;
    struct sockaddr_in source;

    const uint8_t content[] = { 'A', 'B', 'C' };

    TEST_ASSERT_EQUAL_INT(0, testStartReceiver());

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&data, 0, content, sizeof(content)));

    TEST_ASSERT_EQUAL_INT(
        0, encodePacket(&data, buffer, sizeof(buffer), &packet_size));

    /* Damage the packet without updating its checksum. */
    buffer[HEADER_SIZE] ^= 0x01;

    TEST_ASSERT_EQUAL_INT(
        (int)packet_size,
        (int)sendto(test_relay_fd, buffer, packet_size, 0,
                    (struct sockaddr *)&test_receiver_addr,
                    sizeof(test_receiver_addr)));

    /*
     * A corrupted packet must not produce an ACK.
     */
    uint8_t ack_buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_TRUE(
        testRecvDatagram(ack_buffer, sizeof(ack_buffer),
                         &source, 200) < 0);

    /* Send the intact packet and verify it is acknowledged. */
    TEST_ASSERT_EQUAL_INT(0, testSendPacket(&data));
    TEST_ASSERT_EQUAL_INT(0, testReceiveAck(1));

    TEST_ASSERT_EQUAL_INT(0, testSendFinAndFinish(1));
}

/*
 * Out-of-order DATA must not advance the receiver or be written.
 * The receiver should ACK the next expected sequence number.
 */
void test_runReceiver_outOfOrderData(void)
{
    const uint8_t first[] = { 'A' };
    const uint8_t second[] = { 'B' };
    Packet data;

    TEST_ASSERT_EQUAL_INT(0, testStartReceiver());

    /* Sequence 1 arrives before sequence 0. */
    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&data, 1, second, sizeof(second)));
    TEST_ASSERT_EQUAL_INT(0, testSendPacket(&data));
    TEST_ASSERT_EQUAL_INT(0, testReceiveAck(0));

    /* Sequence 0 is accepted. */
    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&data, 0, first, sizeof(first)));
    TEST_ASSERT_EQUAL_INT(0, testSendPacket(&data));
    TEST_ASSERT_EQUAL_INT(0, testReceiveAck(1));

    /*
     * Sequence 1 can now be accepted.
     */
    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&data, 1, second, sizeof(second)));
    TEST_ASSERT_EQUAL_INT(0, testSendPacket(&data));
    TEST_ASSERT_EQUAL_INT(0, testReceiveAck(2));

    TEST_ASSERT_EQUAL_INT(0, testSendFinAndFinish(2));

    int status;
    TEST_ASSERT_EQUAL_INT(
        test_receiver_pid,
        waitpid(test_receiver_pid, &status, 0));

    TEST_ASSERT_TRUE(WIFEXITED(status));
    TEST_ASSERT_EQUAL_INT(0, WEXITSTATUS(status));
    test_receiver_pid = -1;

    FILE *file = fopen(test_output_path, "rb");
    TEST_ASSERT_NOT_NULL(file);

    uint8_t actual[2];
    TEST_ASSERT_EQUAL_size_t(2, fread(actual, 1, sizeof(actual), file));
    TEST_ASSERT_EQUAL_UINT8('A', actual[0]);
    TEST_ASSERT_EQUAL_UINT8('B', actual[1]);
    TEST_ASSERT_EQUAL_INT(EOF, fgetc(file));
    TEST_ASSERT_EQUAL_INT(0, fclose(file));
}


/*
 * A complete transfer with packet loss, corruption, and duplication.
 * 
 */

/* Keep the test bounded and reproducible. */
#define TEST_DATA_SIZE (MAX_PAYLOAD * 100U + 37U)
#define TEST_QUEUE_CAPACITY 512U
#define TEST_RELAY_TIMEOUT_MS 25U
#define TEST_MAX_STEPS 2000000U

typedef struct {
    uint8_t data[MAX_PACKET_SIZE];
    size_t length;
} TestDatagram;

typedef struct {
    TestDatagram items[TEST_QUEUE_CAPACITY];
    size_t head;
    size_t count;
} TestChannel;

typedef struct {
    unsigned int dropped;
    unsigned int corrupted;
    unsigned int duplicated;
} TestFaultStats;

/* Two independent in-memory channels. */
static TestChannel data_channel;
static TestChannel ack_channel;

static TestFaultStats data_faults;
static TestFaultStats ack_faults;

/* Queue one datagram without applying additional faults. */
static int testQueuePush(TestChannel *channel,
                         const uint8_t *data,
                         size_t length)
{
    size_t index;

    if (channel == NULL || data == NULL ||
        length > MAX_PACKET_SIZE ||
        channel->count >= TEST_QUEUE_CAPACITY) {
        return 1;
    }

    index = (channel->head + channel->count) %
            TEST_QUEUE_CAPACITY;

    memcpy(channel->items[index].data, data, length);
    channel->items[index].length = length;
    channel->count++;

    return 0;
}

/* Remove one datagram from a channel. */
static int testQueuePop(TestChannel *channel,
                        TestDatagram *datagram)
{
    if (channel == NULL || datagram == NULL ||
        channel->count == 0U) {
        return 1;
    }

    *datagram = channel->items[channel->head];

    channel->head =
        (channel->head + 1U) % TEST_QUEUE_CAPACITY;

    channel->count--;

    return 0;
}

/*
 * Simulate a lossy, corrupting, duplicating network.
 *
 * Drop, corruption, and duplication are independent events,
 * each with probability 20%.
 *
 * A corrupted datagram has one randomly selected bit flipped.
 */
static int testTransmit(TestChannel *channel,
                        const uint8_t *data,
                        size_t length,
                        TestFaultStats *stats)
{
    uint8_t damaged[MAX_PACKET_SIZE];
    int drop;
    int corrupt;
    int duplicate;

    if (channel == NULL || data == NULL || stats == NULL ||
        length == 0U || length > MAX_PACKET_SIZE) {
        return 1;
    }

    /* One independent random trial per fault type. */
    drop = (rand() % 100) < 20;
    corrupt = (rand() % 100) < 20;
    duplicate = (rand() % 100) < 20;

    if (drop) {
        stats->dropped++;
        return 0;
    }

    memcpy(damaged, data, length);

    if (corrupt) {
        size_t byte_index = (size_t)rand() % length;
        unsigned int bit_index = (unsigned int)rand() % 8U;

        damaged[byte_index] ^=
            (uint8_t)(1U << bit_index);

        stats->corrupted++;
    }

    if (testQueuePush(channel, damaged, length) != 0) {
        return 1;
    }

    if (duplicate) {
        if (testQueuePush(channel, damaged, length) != 0) {
            return 1;
        }

        stats->duplicated++;
    }

    return 0;
}

static int testQueueEncodedPacket(
    TestChannel *channel,
    const Packet *packet,
    TestFaultStats *stats)
{
    uint8_t encoded[MAX_PACKET_SIZE];
    size_t encoded_length = 0;

    if (encodePacket(packet, encoded, sizeof(encoded),
                     &encoded_length) != 0) {
        return 1;
    }

    return testTransmit(
        channel,
        encoded,
        encoded_length,
        stats
    );
}

/*
 * Process one datagram from the sender-to-receiver channel.
 *
 * Valid in-order DATA is copied into the simulated destination file.
 * The receiver's cumulative ACK is transmitted through the reverse
 * channel, where it is subject to its own faults.
 */
static int testProcessData(TestChannel *channel,
                           ReceiverState *receiver,
                           uint8_t *output,
                           size_t output_capacity,
                           size_t *output_length,
                           uint64_t now_ms)
{
    TestDatagram datagram;
    Packet packet;
    Packet ack;
    uint32_t expected_before;

    if (testQueuePop(channel, &datagram) != 0) {
        return 0; /* No datagram available. */
    }

    /* Corrupted or malformed packets are discarded. */
    if (decodePacket(datagram.data, datagram.length,
                     &packet) != 0) {
        return 0;
    }

    if (packet.type == PACKET_DATA) {
        expected_before = receiver->expectedseqnum;

        if (receiverHandleData(receiver, &packet,
                               now_ms, &ack) != 0) {
            return 1;
        }

        /*
         * Write only a packet that was expected before handling it.
         * This models the receiver application writing accepted bytes.
         */
        if (packet.seq == expected_before) {
            if ((size_t)packet.length >
                output_capacity - *output_length) {
                return 1;
            }

            memcpy(output + *output_length,
                   packet.payload, packet.length);

            *output_length += packet.length;
        }
    } else if (packet.type == PACKET_FIN) {
        if (receiverHandleFin(receiver, &packet,
                              now_ms, &ack) != 0) {
            return 1;
        }
    } else {
        return 0; /* ACKs are not receiver-bound data. */
    }

    return testQueueEncodedPacket(&ack_channel, &ack, &ack_faults);
}

/* Process one reverse-channel datagram at the sender. */
static int testProcessAck(TestChannel *channel,
                          SenderState *sender,
                          uint64_t now_ms)
{
    TestDatagram datagram;
    Packet packet;

    if (testQueuePop(channel, &datagram) != 0) {
        return 0;
    }

    if (decodePacket(datagram.data, datagram.length,
                     &packet) != 0) {
        return 0;
    }

    if (packet.type != PACKET_ACK) {
        return 0;
    }

    if (sender->fin_sent && !sender->fin_acked) {
        /*
         * Ignore unrelated ACKs, including stale DATA ACKs.
         */
        (void)senderHandleFinAck(sender, packet.seq, now_ms);
    } else {
        (void)senderHandleAck(sender, packet.seq, now_ms);
    }

    return 0;
}

void test_completeTransfer_withLossCorruptionAndDuplication(void)
{
    static uint8_t source[TEST_DATA_SIZE];
    static uint8_t destination[TEST_DATA_SIZE];

    SenderState sender;
    ReceiverState receiver;

    size_t source_offset = 0U;
    size_t destination_length = 0U;

    uint64_t now_ms = 0U;
    unsigned int steps;

    memset(&data_channel, 0, sizeof(data_channel));
    memset(&ack_channel, 0, sizeof(ack_channel));
    memset(&data_faults, 0, sizeof(data_faults));
    memset(&ack_faults, 0, sizeof(ack_faults));
    memset(destination, 0, sizeof(destination));

    /*
     * Fixed seed: the same packet-fault sequence is generated
     * on each run with the same C library implementation.
     */
    srand(0x5255U);

    /* Use a nontrivial binary payload with varied byte values. */
    for (size_t i = 0U; i < sizeof(source); i++) {
        source[i] = (uint8_t)((i * 37U + i / 7U + 19U) % 256U);
    }

    TEST_ASSERT_EQUAL_INT(
        0, senderInit(&sender, 8U, TEST_RELAY_TIMEOUT_MS));

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    /*
     * Drive both state machines until FIN is acknowledged.
     * Time advances deterministically rather than using a clock.
     */
    for (steps = 0U;
         steps < TEST_MAX_STEPS && !senderFinished(&sender);
         steps++, now_ms++) {

        /*
         * Fill the Go-Back-N window with DATA packets.
         */
        while (source_offset < sizeof(source) &&
               sender.nextseqnum - sender.base < sender.window) {

            size_t remaining = sizeof(source) - source_offset;
            uint16_t chunk_length =
                (uint16_t)(remaining > MAX_PAYLOAD
                           ? MAX_PAYLOAD : remaining);

            Packet packet;

            TEST_ASSERT_EQUAL_INT(
                0,
                senderSendData(&sender,
                               source + source_offset,
                               chunk_length,
                               now_ms,
                               &packet));

            TEST_ASSERT_EQUAL_INT(
                0,
                testQueueEncodedPacket(&data_channel, &packet, &data_faults));

            source_offset += chunk_length;
        }

        /*
         * Process at most one DATA/FIN and one ACK per iteration.
         * This permits queued duplicates and reordering caused by
         * retransmission to be exercised.
         */
        TEST_ASSERT_EQUAL_INT(
            0,
            testProcessData(&data_channel, &receiver,
                            destination, sizeof(destination),
                            &destination_length, now_ms));

        TEST_ASSERT_EQUAL_INT(
            0,
            testProcessAck(&ack_channel, &sender, now_ms));

        /*
         * Send FIN after all source bytes have been queued and all
         * DATA packets have been acknowledged.
         */
        if (source_offset == sizeof(source) &&
            sender.base == sender.nextseqnum &&
            !sender.fin_sent) {

            Packet fin;

            TEST_ASSERT_EQUAL_INT(
                0, senderSendFin(&sender, now_ms, &fin));

            TEST_ASSERT_EQUAL_INT(
                0, testQueueEncodedPacket(&data_channel, &fin, &data_faults));
        }

        /*
         * Expire the oldest outstanding DATA timer and retransmit
         * all unacknowledged DATA packets.
         */
        if (sender.timer_running &&
            now_ms >= sender.timer_start &&
            now_ms - sender.timer_start >= sender.timeout_ms) {

            if (sender.base < sender.nextseqnum) {
                Packet retransmissions[MAX_WINDOW];
                size_t retransmission_count = 0U;

                TEST_ASSERT_EQUAL_INT(
                    0,
                    senderHandleTimeout(&sender, now_ms,
                                        retransmissions,
                                        &retransmission_count));

                for (size_t i = 0U;
                     i < retransmission_count; i++) {
                    TEST_ASSERT_EQUAL_INT(
                        0,
                        testQueueEncodedPacket(&data_channel,
                                       &retransmissions[i],
                                       &data_faults));
                }
            } else if (sender.fin_sent && !sender.fin_acked) {
                /*
                 * FIN is not included in senderHandleTimeout's DATA
                 * retransmission window, so retry it separately.
                 */
                TEST_ASSERT_EQUAL_INT(
                    0,
                    testQueueEncodedPacket(&data_channel,
                                   &sender.fin, &data_faults));

                sender.timer_start = now_ms;
                sender.timer_running = 1;
            }
        }
    }

    /* Fail explicitly if the transfer did not complete. */
    TEST_ASSERT_TRUE_MESSAGE(
        senderFinished(&sender),
        "Transfer did not complete before the step limit");

    TEST_ASSERT_TRUE(receiver.finished);

    /* Every source byte must have been delivered, exactly once. */
    TEST_ASSERT_EQUAL_UINT(sizeof(source), source_offset);
    TEST_ASSERT_EQUAL_UINT(sizeof(source), destination_length);

    TEST_ASSERT_EQUAL_MEMORY(
        source, destination, sizeof(source));

    /*
     * With this fixed seed and sufficiently many packets, the test
     * should exercise each of the three fault types in both directions.
     */
    TEST_ASSERT_TRUE(data_faults.dropped > 0U);
    TEST_ASSERT_TRUE(data_faults.corrupted > 0U);
    TEST_ASSERT_TRUE(data_faults.duplicated > 0U);

    TEST_ASSERT_TRUE(ack_faults.dropped > 0U);
    TEST_ASSERT_TRUE(ack_faults.corrupted > 0U);
    TEST_ASSERT_TRUE(ack_faults.duplicated > 0U);
}




 

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_packetChecksum_empty);
  RUN_TEST(test_packetChecksum_singleByte);
  RUN_TEST(test_packetChecksum_twoBytes);
  RUN_TEST(test_packetChecksum_fourBytes);
  RUN_TEST(test_packetChecksum_oddLength);
  RUN_TEST(test_packetChecksum_oddLength);

    RUN_TEST(test_makeDataPacket_validPacket);
    RUN_TEST(test_makeDataPacket_emptyPayload);
    RUN_TEST(test_makeDataPacket_maxPayload);
    RUN_TEST(test_makeDataPacket_nullPacket);
    RUN_TEST(test_makeDataPacket_nullDataWithNonzeroLength);
    RUN_TEST(test_makeDataPacket_payloadTooLarge);
    RUN_TEST(test_makeDataPacket_zeroLengthWithNonNullData);

    RUN_TEST(test_makeAckPacket_validPacket);
    RUN_TEST(test_makeAckPacket_zeroSequenceNumber);
    RUN_TEST(test_makeAckPacket_maxSequenceNumber);
    RUN_TEST(test_makeAckPacket_nullPacket);

    RUN_TEST(test_makeFinPacket_validPacket);
    RUN_TEST(test_makeFinPacket_zeroSequenceNumber);
    RUN_TEST(test_makeFinPacket_maxSequenceNumber);
    RUN_TEST(test_makeFinPacket_nullPacket);

    RUN_TEST(test_encodePacket_dataPacket);
    RUN_TEST(test_encodePacket_ackPacket);
    RUN_TEST(test_encodePacket_finPacket);
    RUN_TEST(test_encodePacket_emptyDataPacket);
    RUN_TEST(test_encodePacket_maxPayload);
    RUN_TEST(test_encodePacket_nullPacket);
    RUN_TEST(test_encodePacket_nullBuffer);
    RUN_TEST(test_encodePacket_nullPacketSize);
    RUN_TEST(test_encodePacket_bufferTooSmall);
    RUN_TEST(test_encodePacket_invalidType);
    RUN_TEST(test_encodePacket_ackWithPayload);
    RUN_TEST(test_encodePacket_finWithPayload);
    RUN_TEST(test_encodePacket_payloadTooLarge);

    RUN_TEST(test_decodePacket_validDataPacket);
    RUN_TEST(test_decodePacket_validAckPacket);
    RUN_TEST(test_decodePacket_validFinPacket);
    RUN_TEST(test_decodePacket_emptyDataPacket);
    RUN_TEST(test_decodePacket_maxPayload);
    RUN_TEST(test_decodePacket_nullBuffer);
    RUN_TEST(test_decodePacket_nullPacket);
    RUN_TEST(test_decodePacket_tooShort);
    RUN_TEST(test_decodePacket_tooLarge);
    RUN_TEST(test_decodePacket_invalidType);
    RUN_TEST(test_decodePacket_nonzeroReservedByte);
    RUN_TEST(test_decodePacket_lengthMismatch);
    RUN_TEST(test_decodePacket_corruptedPayload);
    RUN_TEST(test_decodePacket_corruptedSequenceNumber);
    RUN_TEST(test_decodePacket_ackWithPayload);
    RUN_TEST(test_decodePacket_finWithPayload);

    RUN_TEST(test_senderInit_validArguments);
    RUN_TEST(test_senderInit_minimumWindow);
    RUN_TEST(test_senderInit_maximumWindow);
    RUN_TEST(test_senderInit_nullPointer);
    RUN_TEST(test_senderInit_zeroWindow);
    RUN_TEST(test_senderInit_windowTooLarge);
    RUN_TEST(test_senderInit_zeroTimeout);
    RUN_TEST(test_senderInit_resetsState);

    RUN_TEST(test_senderSendData_firstPacket);
    RUN_TEST(test_senderSendData_storesPacket);
    RUN_TEST(test_senderSendData_multiplePackets);
    RUN_TEST(test_senderSendData_nullSender);
    RUN_TEST(test_senderSendData_nullData);
    RUN_TEST(test_senderSendData_nullPacket);
    RUN_TEST(test_senderSendData_zeroLength);
    RUN_TEST(test_senderSendData_payloadTooLarge);
    RUN_TEST(test_senderSendData_fullWindow);
    RUN_TEST(test_senderSendData_afterFinSent);
    RUN_TEST(test_senderSendData_afterFinAcked);

    RUN_TEST(test_senderHandleAck_advancesBase);
    RUN_TEST(test_senderHandleAck_duplicateAck);
    RUN_TEST(test_senderHandleAck_staleAck);
    RUN_TEST(test_senderHandleAck_ackBeyondNext);
    RUN_TEST(test_senderHandleAck_allDataAcknowledged);
    RUN_TEST(test_senderHandleAck_noOutstandingData);
    RUN_TEST(test_senderHandleAck_nullSender);
    RUN_TEST(test_senderHandleAck_resetsTimeoutCount);

    RUN_TEST(test_senderHandleTimeout_retransmitsAllOutstanding);
    RUN_TEST(test_senderHandleTimeout_retransmitsUnacknowledgedOnly);
    RUN_TEST(test_senderHandleTimeout_restartsTimer);
    RUN_TEST(test_senderHandleTimeout_incrementsTimeoutCount);
    RUN_TEST(test_senderHandleTimeout_timerNotExpired);
    RUN_TEST(test_senderHandleTimeout_timerNotRunning);
    RUN_TEST(test_senderHandleTimeout_nullSender);
    RUN_TEST(test_senderHandleTimeout_nullPackets);
    RUN_TEST(test_senderHandleTimeout_nullCount);
    RUN_TEST(test_senderHandleTimeout_timeWentBackward);
    RUN_TEST(test_senderHandleTimeout_exactExpiration);
    RUN_TEST(test_senderHandleTimeout_noOutstandingPackets);
    RUN_TEST(test_senderHandleTimeout_exceedsConfiguredWindow);


    RUN_TEST(test_getTimeMs_returns_nonzero);
    RUN_TEST(test_getTimeMs_is_monotonic);
    RUN_TEST(test_getTimeMs_does_not_decrease_over_time);
    RUN_TEST(test_getTimeMs_reasonable_timestamp);

    RUN_TEST(test_senderTimerDue_null_sender);
    RUN_TEST(test_senderTimerDue_null_remaining);
    RUN_TEST(test_senderTimerDue_timer_not_running);
    RUN_TEST(test_senderTimerDue_full_timeout_remaining);
    RUN_TEST(test_senderTimerDue_partial_timeout_remaining);
    RUN_TEST(test_senderTimerDue_timer_expired);
    RUN_TEST(test_senderTimerDue_timer_expired_long_ago);
    RUN_TEST(test_senderTimerDue_time_before_timer_start);
    RUN_TEST(test_senderTimerDue_one_ms_before_expiration);
    RUN_TEST(test_senderTimerDue_zero_timeout);

    RUN_TEST(test_senderSendFin_null_sender);
    RUN_TEST(test_senderSendFin_null_packet);
    RUN_TEST(test_senderSendFin_unacknowledged_data);
    RUN_TEST(test_senderSendFin_success);
    RUN_TEST(test_senderSendFin_empty_file);
    RUN_TEST(test_senderSendFin_already_sent);
    RUN_TEST(test_senderSendFin_does_not_modify_state_on_failure);

    RUN_TEST(test_senderHandleFinAck_null_sender);
    RUN_TEST(test_senderHandleFinAck_fin_not_sent);
    RUN_TEST(test_senderHandleFinAck_already_acknowledged);
    RUN_TEST(test_senderHandleFinAck_wrong_ack_too_small);
    RUN_TEST(test_senderHandleFinAck_wrong_ack_too_large);
    RUN_TEST(test_senderHandleFinAck_success);
    RUN_TEST(test_senderHandleFinAck_empty_file);
    RUN_TEST(test_senderHandleFinAck_timer_already_stopped);

    RUN_TEST(test_senderFinished_null_sender);
    RUN_TEST(test_senderFinished_no_packets_sent);
    RUN_TEST(test_senderFinished_data_not_fully_acknowledged);
    RUN_TEST(test_senderFinished_fin_not_sent);
    RUN_TEST(test_senderFinished_fin_not_acknowledged);
    RUN_TEST(test_senderFinished_success);
    RUN_TEST(test_senderFinished_empty_file_complete);
    RUN_TEST(test_senderFinished_data_acknowledged_but_fin_not_sent);

    RUN_TEST(test_receiverInit_null_receiver);
    RUN_TEST(test_receiverInit_success);
    RUN_TEST(test_receiverInit_resets_existing_state);
    RUN_TEST(test_receiverInit_sets_activity_timestamp);

    RUN_TEST(test_receiverHandleData_expectedPacket);
    RUN_TEST(test_receiverHandleData_multipleInOrderPackets);
    RUN_TEST(test_receiverHandleData_duplicatePacket);
    RUN_TEST(test_receiverHandleData_outOfOrderPacket);
    RUN_TEST(test_receiverHandleData_missingPacket);
    RUN_TEST(test_receiverHandleData_zeroLengthPayload);
    RUN_TEST(test_receiverHandleData_maxPayload);
    RUN_TEST(test_receiverHandleData_nullReceiver);
    RUN_TEST(test_receiverHandleData_nullPacket);
    RUN_TEST(test_receiverHandleData_nullAck);
    RUN_TEST(test_receiverHandleData_nonDataPacket);
    RUN_TEST(test_receiverHandleData_oversizedPayload);
    RUN_TEST(test_receiverHandleData_receiverFinished);
    RUN_TEST(test_receiverHandleData_repeatedOutOfOrderPackets);    

    RUN_TEST(test_receiverHandleFin_nullReceiver);
    RUN_TEST(test_receiverHandleFin_nullPacket);
    RUN_TEST(test_receiverHandleFin_nullAck);
    RUN_TEST(test_receiverHandleFin_nonFinPacket);
    RUN_TEST(test_receiverHandleFin_nonzeroPayload);
    RUN_TEST(test_receiverHandleFin_expectedSequence);
    RUN_TEST(test_receiverHandleFin_emptyFile);
    RUN_TEST(test_receiverHandleFin_outOfOrderSequence);
    RUN_TEST(test_receiverHandleFin_earlyFin);
    RUN_TEST(test_receiverHandleFin_duplicateFin);
    RUN_TEST(test_receiverHandleFin_unexpectedFinAfterCompletion);
    RUN_TEST(test_receiverHandleFin_activityTimestamp);

    RUN_TEST(test_receiverTimerExpired_nullReceiver);
    RUN_TEST(test_receiverTimerExpired_noElapsedTime);
    RUN_TEST(test_receiverTimerExpired_lessThanTimeout);
    RUN_TEST(test_receiverTimerExpired_exactlyAtTimeout);
    RUN_TEST(test_receiverTimerExpired_pastTimeout);
    RUN_TEST(test_receiverTimerExpired_timeBeforeLastActivity);
    RUN_TEST(test_receiverTimerExpired_zeroLastActivity);
    RUN_TEST(test_receiverTimerExpired_largeTimestamps);
    RUN_TEST(test_receiverTimerExpired_doesNotModifyState);

    RUN_TEST(test_resolveAddr_validIPv4);
    RUN_TEST(test_resolveAddr_validHostname);
    RUN_TEST(test_resolveAddr_invalidHostname);
    RUN_TEST(test_resolveAddr_nullHost);
    RUN_TEST(test_resolveAddr_nullPort);
    RUN_TEST(test_resolveAddr_nullResults);
    RUN_TEST(test_resolveAddr_failureClearsResults);
    RUN_TEST(test_resolveAddr_numericService);

    RUN_TEST(test_createSocket_validUDPIPv4);
    RUN_TEST(test_createSocket_validUDPIPv6);
    RUN_TEST(test_createSocket_nullAddress);
    RUN_TEST(test_createSocket_invalidFamily);
    RUN_TEST(test_createSocket_invalidSocketType);
    RUN_TEST(test_createSocket_invalidProtocol);

    RUN_TEST(test_registerReceiver_success);
    RUN_TEST(test_registerReceiver_relayError);
    RUN_TEST(test_registerReceiver_nullRelay);
    RUN_TEST(test_registerReceiver_nullSession);
    RUN_TEST(test_registerReceiver_invalidSessionCharacter);
    RUN_TEST(test_registerReceiver_sessionTooLong);
    RUN_TEST(test_registerReceiver_invalidSocket);
    RUN_TEST(test_registerReceiver_ipv6_address_check);
    RUN_TEST(test_registerReceiver_fails_after_five_attempts);

    RUN_TEST(test_registerSender_success);
    RUN_TEST(test_registerSender_relayError);
    RUN_TEST(test_registerSender_nullRelay);
    RUN_TEST(test_registerSender_nullSession);
    RUN_TEST(test_registerSender_invalidSession);
    RUN_TEST(test_registerSender_sessionTooLong);
    RUN_TEST(test_registerSender_negativeLoss);
    RUN_TEST(test_registerSender_corruptTooHigh);
    RUN_TEST(test_registerSender_dupTooHigh);
    RUN_TEST(test_registerSender_invalidSocket);
    RUN_TEST(test_registerSender_ipv6_success);
    RUN_TEST(test_registerSender_wrongRelayAddress);

    RUN_TEST(test_runSender_nullRelay_returnsFailure);
    RUN_TEST(test_runSender_nullFilename_returnsFailure);
    RUN_TEST(test_runSender_zeroWindow_returnsFailure);
    RUN_TEST(test_runSender_windowAboveMaximum_returnsFailure);
    RUN_TEST(test_runSender_zeroTimeout_returnsFailure);
    RUN_TEST(test_runSender_missingFile_returnsFailure);
    RUN_TEST(test_runSender_invalidPort_returnsFailure);    
    RUN_TEST(test_runSender_relayRejectsRegistration_returnsFailure);
    
    RUN_TEST(test_runReceiver_validDataAndFin);
    RUN_TEST(test_runReceiver_emptyFile);
    RUN_TEST(test_runReceiver_discardsCorruptedPacket);
    RUN_TEST(test_runReceiver_outOfOrderData);  

    RUN_TEST(test_completeTransfer_withLossCorruptionAndDuplication);

  return UNITY_END();
}

```

## Scripts Files
Report generated on 10/10/2026 at 03:47:19


---

## End of Report

SHA-256 Hash of the report: ee49b0b1803783192ee6285fdeccac1082acc4de012367a8ff87692cca7110ed

Do not edit the generated report. Any changes will be reported as academic dishonesty

---
## GitHub Info
- GitHub repo name: 2213amanda/cs525-p2
- The repository visibility is public.
- The workflow was triggered by 2213amanda
