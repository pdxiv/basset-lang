10 REM Test null/empty DATA items
20 REM According to Microsoft BASIC spec, empty values default to 0 or ""
30 REM
40 REM Test 1: Null in middle
50 DATA 1,,3
60 READ A, B, C
70 IF A <> 1 THEN PRINT "FAIL: Expected A=1, got"; A: STOP
80 IF B <> 0 THEN PRINT "FAIL: Expected B=0, got"; B: STOP
90 IF C <> 3 THEN PRINT "FAIL: Expected C=3, got"; C: STOP
100 REM
110 REM Test 2: Multiple nulls in a row
120 DATA 2,,,5
130 READ D, E, F, G
140 IF D <> 2 THEN PRINT "FAIL: Expected D=2, got"; D: STOP
150 IF E <> 0 THEN PRINT "FAIL: Expected E=0, got"; E: STOP
160 IF F <> 0 THEN PRINT "FAIL: Expected F=0, got"; F: STOP
170 IF G <> 5 THEN PRINT "FAIL: Expected G=5, got"; G: STOP
180 REM
190 REM Test 3: Trailing null
200 DATA 7,
210 READ H, I
220 IF H <> 7 THEN PRINT "FAIL: Expected H=7, got"; H: STOP
230 IF I <> 0 THEN PRINT "FAIL: Expected I=0, got"; I: STOP
240 REM
250 REM Test 4: Null with strings
260 DATA "X",,"Z"
270 READ J$, K$, L$
280 IF J$ <> "X" THEN PRINT "FAIL: Expected J$=X": STOP
290 IF K$ <> "" THEN PRINT "FAIL: Expected K$ empty": STOP
300 IF L$ <> "Z" THEN PRINT "FAIL: Expected L$=Z": STOP
310 REM
320 PRINT "PASS"
