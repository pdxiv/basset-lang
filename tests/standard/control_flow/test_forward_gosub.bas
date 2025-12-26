10 REM Test forward GOSUB (recommendation #1)
20 REM Verifies compile-time address resolution
30 PRINT "Testing forward GOSUB"
40 GOSUB 200
50 PRINT "Returned from first GOSUB"
60 GOSUB 300
70 PRINT "Returned from second GOSUB"
80 END
200 PRINT "In subroutine at 200"
210 RETURN
300 PRINT "In subroutine at 300"
310 RETURN
