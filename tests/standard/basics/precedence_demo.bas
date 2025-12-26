10 REM Operator precedence demonstration
20 REM Tests that our table-driven parser handles precedence correctly
30 PRINT "Exponentiation (highest):"
40 PRINT "2 ^ 3 ^ 2 = ", 2 ^ 3 ^ 2
50 PRINT "Expected: 512 (right associative)"
60 PRINT ""
70 PRINT "Multiplication and Division:"
80 PRINT "10 * 2 / 5 = ", 10 * 2 / 5
90 PRINT "Expected: 4"
100 PRINT ""
110 PRINT "Addition and Subtraction:"
120 PRINT "10 + 5 - 3 = ", 10 + 5 - 3
130 PRINT "Expected: 12"
140 PRINT ""
150 PRINT "Mixed precedence:"
160 PRINT "2 + 3 * 4 ^ 2 = ", 2 + 3 * 4 ^ 2
170 PRINT "Expected: 50"
180 PRINT ""
190 PRINT "Comparison operators:"
200 PRINT "5 < 10 AND 3 > 1 = ", 5 < 10 AND 3 > 1
210 PRINT "Expected: 1 (true)"
220 END
