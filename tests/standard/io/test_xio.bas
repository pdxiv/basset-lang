10 REM Test XIO file operations
20 PRINT "Testing XIO file operations..."
30 REM XIO 8 = OPEN for output
40 XIO 8, #1, 0, 0, "xio_test.txt"
50 PRINT "Opened file with XIO"
60 REM Use PUT to write data
70 PUT #1,72
80 PUT #1,73
90 PUT #1,10
100 REM XIO 12 = CLOSE
110 XIO 12, #1, 0, 0, ""
120 PRINT "Closed file with XIO"
130 REM XIO 3 = OPEN for input
140 XIO 3, #1, 0, 0, "xio_test.txt"
150 PRINT "Reopened file for reading"
160 GET #1, A
170 GET #1, B
180 PRINT "Read bytes:"; A; B
190 XIO 12, #1, 0, 0, ""
200 PRINT
210 REM XIO 34 = DELETE
220 PRINT "Deleting test file..."
230 XIO 34, #1, 0, 0, "xio_test.txt"
240 PRINT
250 PRINT "=== XIO Test Passed ==="
