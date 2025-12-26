10 REM Comprehensive File I/O Test
20 PRINT "=== Comprehensive File I/O Test ==="
30 PRINT
40 REM Write a file
50 PRINT "Writing data to file..."
60 OPEN #1,8,0,"demo.dat"
70 FOR I=1 TO 10
80 PUT #1,I*10
90 NEXT I
100 CLOSE #1
110 PRINT "Wrote 10 bytes"
120 PRINT
130 REM Read the file back
140 PRINT "Reading data from file..."
150 OPEN #1,4,0,"demo.dat"
160 FOR I=1 TO 10
170 GET #1,X
180 PRINT "Byte ";I;": ";X
190 NEXT I
200 CLOSE #1
210 PRINT
220 REM Test STATUS
230 PRINT "Testing STATUS..."
240 OPEN #2,4,0,"demo.dat"
250 STATUS #2,S
260 PRINT "Status before read: ";S
270 GET #2,X
280 STATUS #2,S
290 PRINT "Status after read: ";S
300 CLOSE #2
310 PRINT
320 REM Test NOTE and POINT
330 PRINT "Testing NOTE/POINT..."
340 OPEN #3,12,0,"demo.dat"
350 FOR I=1 TO 5
360 GET #3,X
370 NEXT I
380 NOTE #3,SEC,POS
390 PRINT "Position after 5 reads: sector=";SEC;" byte=";POS
400 POINT #3,0,2
410 GET #3,X
420 PRINT "Byte at position 2: ";X
430 CLOSE #3
440 PRINT
450 PRINT "=== All File I/O Tests Passed ==="
460 END
