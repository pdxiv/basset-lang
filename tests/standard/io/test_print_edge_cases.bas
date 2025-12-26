10 REM Comprehensive PRINT# edge case testing
20 OPEN #1,8,0,"channel1.txt"
30 REM Test 1: Single value
40 PRINT #1,42
50 REM Test 2: String only
60 PRINT #1,"Hello"
70 REM Test 3: Mixed types
80 PRINT #1,"Value:",100
90 REM Test 4: Multiple PRINT# on same line
100 PRINT #1,1:PRINT #1,2:PRINT #1,3
110 REM Test 5: PRINT# with semicolon (no newline)
120 PRINT #1,"No";:PRINT #1,"Space"
130 REM Test 6: Back to stdout
140 PRINT "Console"
150 REM Test 7: PRINT# again
160 PRINT #1,"Final"
170 CLOSE #1
180 PRINT "Done"
