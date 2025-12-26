10 REM Test unquoted identifiers in DATA statements
20 REM According to Microsoft BASIC spec, unquoted identifiers are stored as strings
30 REM not converted to 0
40 REM
50 REM Test 1: Simple identifiers
60 DATA ABC, DEF, XYZ
70 READ A$, B$, C$
80 IF A$ <> "ABC" THEN PRINT "FAIL: Expected ABC, got"; A$: STOP
90 IF B$ <> "DEF" THEN PRINT "FAIL: Expected DEF, got"; B$: STOP
100 IF C$ <> "XYZ" THEN PRINT "FAIL: Expected XYZ, got"; C$: STOP
110 REM
120 REM Test 2: Mixed with numbers
130 DATA 123, ABC, 456
140 READ D, E$, F
150 IF D <> 123 THEN PRINT "FAIL: Expected 123, got"; D: STOP
160 IF E$ <> "ABC" THEN PRINT "FAIL: Expected ABC, got"; E$: STOP
170 IF F <> 456 THEN PRINT "FAIL: Expected 456, got"; F: STOP
180 REM
190 REM Test 3: Identifier read as number should give 0
200 DATA HELLO
210 READ G
220 IF G <> 0 THEN PRINT "FAIL: Expected 0, got"; G: STOP
230 REM
240 REM Test 4: Lowercase identifier (if tokenizer supports)
250 DATA test
260 READ H$
270 IF H$ <> "test" AND H$ <> "TEST" THEN PRINT "FAIL: Got"; H$: STOP
280 REM
290 PRINT "PASS"
