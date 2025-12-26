10 REM Test READ statement per Microsoft BASIC spec
20 REM
30 REM Test 1: Multiple variables in one READ
40 DATA 10, 20, 30
50 READ A, B, C
60 PRINT "Multiple variables:"
70 PRINT A, B, C
80 PRINT
90 REM Test 2: String and numeric variables
100 DATA "Hello", 42, "World", 3.14
110 READ S1$, N1, S2$, N2
120 PRINT "Mixed types:"
130 PRINT S1$, N1, S2$, N2
140 PRINT
150 REM Test 3: Numeric to string conversion
160 DATA 123, 456
170 READ X$, Y$
180 PRINT "Numeric to string:"
190 PRINT X$, Y$
200 PRINT
210 REM Test 4: Multiple DATA statements treated as one list
220 DATA 1
230 DATA 2
240 DATA 3
250 READ D1, D2, D3
260 PRINT "Sequential DATA statements:"
270 PRINT D1, D2, D3
280 PRINT
290 REM Test 5: RESTORE resets to beginning
300 READ R1
310 PRINT "Read one more value:"
320 PRINT R1
330 RESTORE
340 READ R1, R2
350 PRINT "After RESTORE (back to start):"
360 PRINT R1, R2
370 DATA 999
