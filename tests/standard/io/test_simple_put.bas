10 REM Simple write test
20 OPEN #1,8,0,"simple.dat"
30 I=1
40 PUT #1,I*10
50 I=2
60 PUT #1,I*10
70 CLOSE #1
80 OPEN #1,4,0,"simple.dat"
90 GET #1,X
100 PRINT "First byte: ";X
110 GET #1,Y
120 PRINT "Second byte: ";Y
130 CLOSE #1
140 END
