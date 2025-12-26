10 REM Test TRAP statement
20 TRAP 100
30 PRINT "TRAP set to line 100"
40 REM In a real implementation, division by zero would trigger trap
50 REM For now, just show that TRAP parses correctly
60 TRAP
70 PRINT "TRAP cleared (no argument)"
80 END
100 REM Error handler would go here
110 PRINT "Error trapped!"
120 END
