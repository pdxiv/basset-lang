10 REM Test file reading with OPEN/GET/CLOSE
20 PRINT "Reading from test file..."
30 OPEN #1,4,0,"tests/standard/test_output.txt"
40 FOR I=1 TO 11
50 GET #1,C
60 PRINT "Read char: ";C;" = ";CHR$(C)
70 NEXT I
80 CLOSE #1
90 PRINT "File read successfully"
100 END
