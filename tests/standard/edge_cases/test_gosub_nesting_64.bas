10 REM Test 64-level nested GOSUB limit
20 PRINT "Testing deep GOSUB nesting (64 levels)"
30 C=0
40 GOSUB 1000
50 PRINT "Returned from 64 levels: PASSED"
60 END
1000 C=C+1:IF C<64 THEN GOSUB 1000
1010 IF C=64 THEN PRINT "Reached 64-level depth"
1020 C=C-1:RETURN
