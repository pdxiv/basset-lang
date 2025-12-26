10 REM Test PRINT# to file
20 OPEN #1,8,0,"test_print_data.txt"
30 PRINT #1,10,20,30
40 PRINT #1,"Hello","World"
50 CLOSE #1
60 PRINT "File written successfully"
