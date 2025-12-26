10 REM Test ON GOTO with forward references (recommendation #3)
20 REM Verifies address table optimization
30 PRINT "Testing ON GOTO with forward refs"
40 FOR I = 1 TO 3
50 ON I GOTO 200,300,400
60 PRINT "ERROR: Should not reach line 60"
70 NEXT I
80 PRINT "All ON GOTO forward tests passed"
90 END
200 PRINT "Branch 1"
210 GOTO 70
300 PRINT "Branch 2"
310 GOTO 70
400 PRINT "Branch 3"
410 GOTO 70
