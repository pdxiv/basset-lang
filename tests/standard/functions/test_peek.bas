10 REM Test PEEK function (recommendation #2)
20 REM PEEK was added to fix opcode conflict with SGN
30 PRINT "Testing PEEK function"
40 X = PEEK(0)
50 PRINT "PEEK(0) = ";X
60 Y = PEEK(1024)
70 PRINT "PEEK(1024) = ";Y
80 Z = PEEK(65535)
90 PRINT "PEEK(65535) = ";Z
100 PRINT "PEEK tests passed"
110 END
