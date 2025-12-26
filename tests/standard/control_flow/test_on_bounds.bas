10 REM Test ON with out-of-bounds index
20 PRINT "Testing ON bounds"
30 X=0
40 ON X GOTO 100,200,300
50 PRINT "Index 0: Continued (correct)"
60 X=4
70 ON X GOTO 100,200,300
80 PRINT "Index 4: Continued (correct)"
90 PRINT "Bounds test passed"
100 END
200 PRINT "ERROR: Should not reach branch 1"
210 END
300 PRINT "ERROR: Should not reach branch 2"
310 END
400 PRINT "ERROR: Should not reach branch 3"
410 END
