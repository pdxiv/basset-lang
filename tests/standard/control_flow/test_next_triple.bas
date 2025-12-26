10 REM Test nested NEXT with three variables
20 FOR X=1 TO 2
30 FOR Y=1 TO 2
40 FOR Z=1 TO 2
50 PRINT X;"-";Y;"-";Z
60 NEXT Z,Y,X
70 PRINT "Complete"
80 END
