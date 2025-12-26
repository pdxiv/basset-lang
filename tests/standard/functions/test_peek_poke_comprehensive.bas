10 REM Comprehensive PEEK and POKE tests
20 REM Based on Microsoft BASIC specification
30 REM
40 PRINT "Testing PEEK and POKE functions..."
50 PRINT
60 REM Test 1: Basic POKE and PEEK
70 PRINT "Test 1: Basic POKE and PEEK"
80 POKE 100, 42
90 X = PEEK(100)
100 IF X <> 42 THEN PRINT "FAIL: Expected 42, got"; X: STOP
110 PRINT "  PASS: POKE/PEEK at address 100"
120 PRINT
130 REM Test 2: Boundary values (0-255)
140 PRINT "Test 2: Boundary values"
150 POKE 200, 0
160 IF PEEK(200) <> 0 THEN PRINT "FAIL: PEEK(200) <> 0": STOP
170 POKE 200, 255
180 IF PEEK(200) <> 255 THEN PRINT "FAIL: PEEK(200) <> 255": STOP
190 POKE 200, 128
200 IF PEEK(200) <> 128 THEN PRINT "FAIL: PEEK(200) <> 128": STOP
210 PRINT "  PASS: Boundary values 0, 255, 128"
220 PRINT
230 REM Test 3: Low byte only (values > 255)
240 PRINT "Test 3: Low byte extraction"
250 REM Per Microsoft spec: only low byte is used
260 POKE 300, 256
270 IF PEEK(300) <> 0 THEN PRINT "FAIL: 256 should store as 0": STOP
280 POKE 300, 257
290 IF PEEK(300) <> 1 THEN PRINT "FAIL: 257 should store as 1": STOP
300 POKE 300, 511
310 IF PEEK(300) <> 255 THEN PRINT "FAIL: 511 should store as 255": STOP
320 PRINT "  PASS: Low byte extraction working"
330 PRINT
340 REM Test 4: Multiple locations
350 PRINT "Test 4: Multiple independent locations"
360 POKE 1000, 10
370 POKE 2000, 20
380 POKE 3000, 30
390 IF PEEK(1000) <> 10 THEN PRINT "FAIL: addr 1000": STOP
400 IF PEEK(2000) <> 20 THEN PRINT "FAIL: addr 2000": STOP
410 IF PEEK(3000) <> 30 THEN PRINT "FAIL: addr 3000": STOP
420 PRINT "  PASS: Multiple locations independent"
430 PRINT
440 REM Test 5: Sequential addresses
450 PRINT "Test 5: Sequential memory addresses"
460 POKE 5000, 100
470 POKE 5001, 101
480 POKE 5002, 102
490 IF PEEK(5000) <> 100 THEN PRINT "FAIL: addr 5000": STOP
500 IF PEEK(5001) <> 101 THEN PRINT "FAIL: addr 5001": STOP
510 IF PEEK(5002) <> 102 THEN PRINT "FAIL: addr 5002": STOP
520 PRINT "  PASS: Sequential addresses work"
530 PRINT
540 REM Test 6: Overwriting values
550 PRINT "Test 6: Overwriting existing values"
560 POKE 10000, 50
570 IF PEEK(10000) <> 50 THEN PRINT "FAIL: Initial value": STOP
580 POKE 10000, 75
590 IF PEEK(10000) <> 75 THEN PRINT "FAIL: Updated value": STOP
600 POKE 10000, 0
610 IF PEEK(10000) <> 0 THEN PRINT "FAIL: Cleared value": STOP
620 PRINT "  PASS: Overwriting works correctly"
630 PRINT
640 REM Test 7: High memory addresses
650 PRINT "Test 7: High memory addresses (near 64KB)"
660 POKE 65000, 200
670 IF PEEK(65000) <> 200 THEN PRINT "FAIL: addr 65000": STOP
680 POKE 65535, 255
690 IF PEEK(65535) <> 255 THEN PRINT "FAIL: addr 65535 (max)": STOP
700 PRINT "  PASS: High addresses work"
710 PRINT
720 REM Test 8: Using variables for address and value
730 PRINT "Test 8: Variable address and value"
740 A = 12345
750 V = 77
760 POKE A, V
770 R = PEEK(A)
780 IF R <> V THEN PRINT "FAIL: Variable addressing": STOP
790 PRINT "  PASS: Variable addressing works"
800 PRINT
810 REM Test 9: Expressions in POKE/PEEK
820 PRINT "Test 9: Expressions for address/value"
830 B = 100
840 POKE B + 50, 33 * 2
850 IF PEEK(150) <> 66 THEN PRINT "FAIL: Expression handling": STOP
860 PRINT "  PASS: Expressions work correctly"
870 PRINT
880 REM Test 10: Initial memory state (should be 0)
890 PRINT "Test 10: Initial memory state"
900 REM Fresh addresses should return 0
910 IF PEEK(30000) <> 0 THEN PRINT "FAIL: Uninitialized memory": STOP
920 IF PEEK(40000) <> 0 THEN PRINT "FAIL: Uninitialized memory": STOP
930 PRINT "  PASS: Uninitialized memory is 0"
940 PRINT
950 PRINT "All PEEK/POKE tests PASSED!"
