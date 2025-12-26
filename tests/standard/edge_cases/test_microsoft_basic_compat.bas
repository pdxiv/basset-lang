10 REM Comprehensive Microsoft BASIC compatibility test
20 REM Test CLEAR with parameter
30 CLEAR 1000
40 REM Test all DEFtype statements with various syntaxes
50 DEFINT I-N
60 DEFLNG L
70 DEFSNG S-T
80 DEFDBL D-E
90 DEFSTR A-C
100 REM Variables should work normally (classic BASIC types)
110 I = 42
120 L = 12345
130 S = 3.14159
140 D = 2.71828
150 A$ = "HELLO"
160 REM Test CLS
170 CLS
180 PRINT "All Microsoft BASIC statements parsed successfully!"
190 PRINT "I=";I;" L=";L;" S=";S;" D=";D;" A$=";A$
200 END
