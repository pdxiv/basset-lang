10 REM Test forward GOTO (recommendation #1)
20 REM Verifies compile-time address resolution
30 PRINT "Testing forward GOTO"
40 GOTO 100
50 PRINT "ERROR: Should not reach line 50"
60 END
100 PRINT "Forward GOTO successful"
110 GOTO 200
120 PRINT "ERROR: Should not reach line 120"
130 END
200 PRINT "Second forward GOTO successful"
210 END
