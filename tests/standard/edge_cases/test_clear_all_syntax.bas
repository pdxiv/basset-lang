10 REM Test all CLEAR statement syntaxes
20 REM Basic CLEAR
30 CLEAR
40 PRINT "Test 1: CLEAR - OK"
50 REM CLEAR with leading comma
60 CLEAR ,
70 PRINT "Test 2: CLEAR , - OK"
80 REM CLEAR with memory limit
90 CLEAR ,32768
100 PRINT "Test 3: CLEAR ,32768 - OK"
110 REM CLEAR with stack size only
120 CLEAR ,,2000
130 PRINT "Test 4: CLEAR ,,2000 - OK"
140 REM CLEAR with both parameters
150 CLEAR ,32768,2000
160 PRINT "Test 5: CLEAR ,32768,2000 - OK"
170 PRINT "All CLEAR syntax tests passed!"
180 END
