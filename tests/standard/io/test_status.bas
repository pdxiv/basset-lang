10 REM Test STATUS statement
20 PRINT "Testing STATUS command..."
30 OPEN #1,4,0,"nonexistent.txt"
40 STATUS #1,S
50 PRINT "Status after failed OPEN: ";S
60 CLOSE #1
70 PRINT "Test complete"
80 END
