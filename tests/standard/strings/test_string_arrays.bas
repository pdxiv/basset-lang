10 REM Test 1D string arrays
20 DIM A$(5)
30 A$(0) = "ZERO"
40 A$(1) = "ONE"
50 A$(2) = "TWO"
60 A$(3) = "THREE"
70 A$(4) = "FOUR"
80 A$(5) = "FIVE"
90 PRINT "String array elements:"
100 FOR I = 0 TO 5
110 PRINT "A$("; I; ") = "; A$(I)
120 NEXT I
130 PRINT
140 PRINT "=== String Array Test Passed ==="
