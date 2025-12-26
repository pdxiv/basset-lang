10 REM Test 2D string arrays
20 DIM B$(2,2)
30 B$(0,0) = "A1"
40 B$(0,1) = "A2"
50 B$(0,2) = "A3"
60 B$(1,0) = "B1"
70 B$(1,1) = "B2"
80 B$(1,2) = "B3"
90 B$(2,0) = "C1"
100 B$(2,1) = "C2"
110 B$(2,2) = "C3"
120 PRINT "2D String Array:"
130 FOR I = 0 TO 2
140 FOR J = 0 TO 2
150 PRINT B$(I,J)
160 NEXT J
170 PRINT
180 NEXT I
190 PRINT
200 PRINT "=== 2D String Array Test Passed ==="
