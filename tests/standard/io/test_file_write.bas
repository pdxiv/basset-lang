10 REM Test file writing with OPEN/PUT/CLOSE
20 PRINT "Writing to test file..."
30 OPEN #1,8,0,"test_write_output.txt"
40 FOR I=65 TO 75
50 PUT #1,I
60 NEXT I
70 CLOSE #1
80 PRINT "File written successfully"
90 END
