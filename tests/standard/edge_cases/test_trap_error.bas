10 REM Test TRAP error handling
20 TRAP 100
30 PRINT "Testing division by zero with TRAP..."
40 X = 10 / 0
50 PRINT "This should not print"
60 END
100 REM Error handler
110 PRINT "Error caught by TRAP!"
120 PRINT "Error code in PEEK(195) would be: 11"
130 PRINT "Continuing after error..."
140 PRINT "=== TRAP Test Passed ==="
