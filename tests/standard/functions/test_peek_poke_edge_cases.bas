10 REM PEEK/POKE edge cases and practical examples
20 REM Demonstrates real-world usage patterns
30 PRINT "PEEK/POKE Edge Cases and Examples"
40 PRINT
50 REM Test 1: Memory as data storage
60 PRINT "Test 1: Using memory as simple data storage"
70 REM Store a sequence of values
80 FOR I = 0 TO 9
90   POKE 50000 + I, I * 10
100 NEXT I
110 REM Read them back
120 SUM = 0
130 FOR I = 0 TO 9
140   SUM = SUM + PEEK(50000 + I)
150 NEXT I
160 REM Sum should be 0+10+20+...+90 = 450
170 IF SUM <> 450 THEN PRINT "FAIL: Sum is"; SUM: STOP
180 PRINT "  PASS: Stored and retrieved sequence"
190 PRINT
200 REM Test 2: Using memory as a lookup table
210 PRINT "Test 2: Memory as lookup table"
220 REM Store lookup table at address 60000
230 REM Map: 0->5, 1->15, 2->25, 3->35
240 POKE 60000, 5
250 POKE 60001, 15
260 POKE 60002, 25
270 POKE 60003, 35
280 REM Lookup values
290 IDX = 2
300 VL = PEEK(60000 + IDX)
310 IF VL <> 25 THEN PRINT "FAIL: Lookup failed": STOP
320 PRINT "  PASS: Lookup table works"
330 PRINT
340 REM Test 3: Bit manipulation (low byte only)
350 PRINT "Test 3: Verify low byte extraction"
360 REM Test that values wrap around at 256
370 POKE 1234, 300
380 REM 300 = 256 + 44, so low byte = 44
390 IF PEEK(1234) <> 44 THEN PRINT "FAIL: 300 mod 256": STOP
400 POKE 1234, 1000
410 REM 1000 = 3*256 + 232, so low byte = 232
420 IF PEEK(1234) <> 232 THEN PRINT "FAIL: 1000 mod 256": STOP
430 PRINT "  PASS: Low byte extraction correct"
440 PRINT
450 REM Test 4: Zero page and common locations
460 PRINT "Test 4: Common memory regions"
470 REM Zero page (0-255)
480 POKE 0, 111
490 IF PEEK(0) <> 111 THEN PRINT "FAIL: Zero page": STOP
500 POKE 255, 222
510 IF PEEK(255) <> 222 THEN PRINT "FAIL: End of zero page": STOP
520 REM Mid-range
530 POKE 32768, 128
540 IF PEEK(32768) <> 128 THEN PRINT "FAIL: 32K boundary": STOP
550 PRINT "  PASS: Various memory regions"
560 PRINT
570 REM Test 5: Floating point addresses (converted to integer)
580 PRINT "Test 5: Floating point addresses"
590 REM Per Microsoft spec: float addresses converted to 2-byte int
600 A = 1500.7
610 POKE A, 99
620 REM Should round/truncate to 1500
630 IF PEEK(1500) <> 99 THEN PRINT "FAIL: Float address": STOP
640 PRINT "  PASS: Float addresses converted"
650 PRINT
660 REM Test 6: Negative values (low byte used)
670 PRINT "Test 6: Negative value handling"
680 REM Negative values: only low byte matters
690 REM -1 in two's complement is 0xFF = 255
700 POKE 2000, -1
710 IF PEEK(2000) <> 255 THEN PRINT "FAIL: -1 handling": STOP
720 PRINT "  PASS: Negative values handled"
730 PRINT
740 REM Test 7: Building multi-byte values
750 PRINT "Test 7: Simulating 16-bit storage"
760 REM Store 16-bit value 1234 as two bytes
770 V = 1234
780 LO = V - INT(V / 256) * 256
790 HI = INT(V / 256)
800 POKE 8000, LO
810 POKE 8001, HI
820 REM Read back
830 RLO = PEEK(8000)
840 RHI = PEEK(8001)
850 RESULT = RLO + RHI * 256
860 IF RESULT <> 1234 THEN PRINT "FAIL: 16-bit storage": STOP
870 PRINT "  PASS: Multi-byte storage simulation"
880 PRINT
890 PRINT "All edge case tests PASSED!"
