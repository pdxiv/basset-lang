10 REM Test ERR function - returns error codes
20 REM Test 1: ERR is 0 initially (no error)
30 IF ERR <> 0 THEN PRINT "FAIL: ERR should be 0 initially": END
40 PRINT "PASS: ERR initially 0"
50 REM Test 2: Division by zero (error code 11)
60 TRAP 1000
70 X = 1 / 0
80 PRINT "FAIL: Should have trapped": END
1000 IF ERR <> 11 THEN PRINT "FAIL: Expected ERR=11 (division by zero), got"; ERR: END
1010 PRINT "PASS: Division by zero ERR=11"
1020 REM Test 3: Type mismatch (error code 13)
1030 TRAP 2000
1040 Y = LEN(42)
1050 PRINT "FAIL: Should have trapped": END
2000 IF ERR <> 13 THEN PRINT "FAIL: Expected ERR=13 (type mismatch), got"; ERR: END
2010 PRINT "PASS: Type mismatch ERR=13"
2020 REM Test 4: Array bounds (error code 9)
2030 DIM A(10)
2040 TRAP 3000
2050 Z = A(100)
2060 PRINT "FAIL: Should have trapped": END
3000 IF ERR <> 9 THEN PRINT "FAIL: Expected ERR=9 (subscript range), got"; ERR: END
3010 PRINT "PASS: Array bounds ERR=9"
3020 REM Test 5: OUT OF DATA (error code 4)
3030 DATA 42
3040 READ A
3050 TRAP 4000
3060 READ B
3070 PRINT "FAIL: Should have trapped": END
4000 IF ERR <> 4 THEN PRINT "FAIL: Expected ERR=4 (out of data), got"; ERR: END
4010 PRINT "PASS: Out of data ERR=4"
4020 PRINT "=== All ERR function tests passed ==="
4030 END
