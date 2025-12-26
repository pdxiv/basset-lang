10 REM Test DATA edge cases per Microsoft BASIC spec
20 REM
30 REM Test 1: Strings with commas need quotes
40 DATA "Smith, John"
50 DATA "Doe, Jane"
60 READ N1$
70 READ N2$
80 PRINT "Names with commas:"
90 PRINT N1$
100 PRINT N2$
110 PRINT
120 REM Test 2: Strings with leading/trailing spaces
130 DATA "  spaced  "
140 DATA "normal"
150 READ S1$
160 READ S2$
170 PRINT "Strings with spaces:"
180 PRINT "[";S1$;"]"
190 PRINT "[";S2$;"]"
200 PRINT
210 REM Test 3: REM after DATA requires colon
220 DATA 100
230 DATA 200
240 DATA 300: REM This is a comment
250 READ D1, D2, D3
260 PRINT "Values after DATA with REM:"
270 PRINT D1, D2, D3
280 PRINT
290 REM Test 4: String with colon needs quotes
300 DATA "Time: 10:30"
310 DATA "Regular"
320 READ T1$
330 READ T2$
340 PRINT "String with colon:"
350 PRINT T1$
360 PRINT T2$
370 PRINT
380 REM Test 5: Multiple DATA statements
390 DATA 1, 2
400 DATA 3, 4
410 DATA 5, 6
420 READ A, B, C, D, E, F
430 PRINT "Multiple DATA statements:"
440 PRINT A, B, C, D, E, F
450 PRINT
460 REM Test 6: Numbers and strings
470 DATA 99, "text", 3.14
480 READ Y, Z$, W
490 PRINT "Mixed numeric and string:"
500 PRINT Y, Z$, W


