10 REM Comprehensive test of all new features
20 PRINT "=== Testing Substring Functions ==="
30 A$ = "HELLO WORLD"
40 PRINT "LEFT$(A$,5):"
42 PRINT LEFT$(A$,5)
50 PRINT "RIGHT$(A$,5):"
52 PRINT RIGHT$(A$,5)
60 PRINT "MID$(A$,7,5):"
62 PRINT MID$(A$,7,5)
70 PRINT
80 PRINT "=== Testing String Arrays ==="
90 DIM NAMES$(3)
100 NAMES$(0) = "ALICE"
110 NAMES$(1) = "BOB"
120 NAMES$(2) = "CHARLIE"
130 NAMES$(3) = "DIANA"
140 FOR I = 0 TO 3
145 PRINT "NAMES$("
146 PRINT I
147 PRINT "):"
150 PRINT NAMES$(I)
160 NEXT I
170 PRINT
180 PRINT "=== Testing String Functions with Arrays ==="
190 FOR I = 0 TO 3
200 PRINT LEFT$(NAMES$(I),3)
210 NEXT I
220 PRINT
230 PRINT
240 PRINT "=== Testing 2D String Arrays ==="
250 DIM GRID$(1,1)
260 GRID$(0,0) = "NW"
270 GRID$(0,1) = "NE"
280 GRID$(1,0) = "SW"
290 GRID$(1,1) = "SE"
300 FOR I = 0 TO 1
310 FOR J = 0 TO 1
320 PRINT GRID$(I,J)
330 NEXT J
340 PRINT
350 NEXT I
360 PRINT
370 PRINT "=== All Comprehensive Tests Passed ==="
