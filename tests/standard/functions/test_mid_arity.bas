10 REM Test MID$ function arity validation
20 REM MID$ accepts 2-3 arguments
30 A$ = "HELLO"
40 X$ = MID$(A$, 2)
50 PRINT X$
60 Y$ = MID$(A$, 2, 3)
70 PRINT Y$
