10 REM Test RANDOMIZE statement
20 PRINT "=== RANDOMIZE TEST ==="
30 REM Seed with known value for deterministic output using RND(-1)
40 X=RND(-1)
50 PRINT "After RND(-1):"
60 PRINT "RND(1) = ";RND(1)
70 PRINT "RND(1) = ";RND(1)
80 REM Test RANDOMIZE with numeric argument
90 RANDOMIZE 42
100 PRINT "After RANDOMIZE 42:"
110 PRINT "RND(1) = ";RND(1)
120 PRINT "RND(1) = ";RND(1)
130 REM Test RANDOMIZE with expression
140 RANDOMIZE 5*2
150 PRINT "After RANDOMIZE 5*2:"
160 PRINT "RND(1) = ";RND(1)
170 PRINT "RND(1) = ";RND(1)
180 END
