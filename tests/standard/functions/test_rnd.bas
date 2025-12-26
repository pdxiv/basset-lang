10 REM Test RND function with seeding
20 PRINT "=== RND FUNCTION TEST ==="
30 REM Seed with -1 to get predictable sequence
40 X=RND(-1)
50 PRINT "After RND(-1):"
60 PRINT "RND(0) = ";RND(0)
70 PRINT "RND(1) = ";RND(1)
80 PRINT "RND(1) = ";RND(1)
90 PRINT "RND(1) = ";RND(1)
100 REM Test RND(0) returns last value
110 PRINT "RND(0) = ";RND(0)
120 REM Reseed with different value
130 X=RND(-5)
140 PRINT "After RND(-5):"
150 PRINT "RND(0) = ";RND(0)
160 PRINT "RND(1) = ";RND(1)
170 PRINT "RND(1) = ";RND(1)
180 END
