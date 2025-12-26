10 REM Test PRINT# with multiple statements like adventure.bas line 720
20 OPEN #1,8,0,"channel1.txt"
30 SF=1:LX=2:DF=3:R=4
40 PRINT #1,SF,LX,DF,R:PRINT "Done with first PRINT#"
50 CLOSE #1
60 PRINT "Test complete"
