10 REM Test PRINT# channel switching - write to multiple files
20 OPEN #1,8,0,"channel1.txt"
30 OPEN #2,8,0,"channel2.txt"
40 REM Write to channel 1
50 PRINT #1,"File 1 Line 1"
60 PRINT #1,100,200
70 REM Write to channel 2
80 PRINT #2,"File 2 Line 1"
90 PRINT #2,300,400
100 REM Write more to channel 1
110 PRINT #1,"File 1 Line 2"
120 REM Write to stdout
130 PRINT "Console output"
140 REM Write more to channel 2
150 PRINT #2,"File 2 Line 2"
160 CLOSE #1
170 CLOSE #2
180 PRINT "Test complete - check channel1.txt and channel2.txt"
