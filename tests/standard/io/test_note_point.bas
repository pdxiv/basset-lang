10 REM Test NOTE and POINT statements
20 PRINT "Testing NOTE/POINT..."
30 OPEN #1,8,0,"test_note.txt"
40 FOR I=65 TO 90
50 PUT #1,I
60 NEXT I
70 NOTE #1,SEC,POS
80 PRINT "Position after write: sector=";SEC;" pos=";POS
90 POINT #1,0,10
100 PUT #1,88
110 CLOSE #1
120 PRINT "Test complete"
130 END
