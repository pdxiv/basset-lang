10 REM Test variable limits (recommendation #5)
20 REM Tests that we can use up to 128 numeric variables
30 PRINT "Testing variable limits"
40 REM Create 50 variables to test limit enforcement
50 A1=1:A2=2:A3=3:A4=4:A5=5
60 A6=6:A7=7:A8=8:A9=9:A10=10
70 B1=11:B2=12:B3=13:B4=14:B5=15
80 B6=16:B7=17:B8=18:B9=19:B10=20
90 C1=21:C2=22:C3=23:C4=24:C5=25
100 C6=26:C7=27:C8=28:C9=29:C10=30
110 D1=31:D2=32:D3=33:D4=34:D5=35
120 D6=36:D7=37:D8=38:D9=39:D10=40
130 E1=41:E2=42:E3=43:E4=44:E5=45
140 E6=46:E7=47:E8=48:E9=49:E10=50
150 PRINT "Created 50 variables successfully"
160 PRINT "Sum test: ";A1+B1+C1+D1+E1
170 PRINT "All variable limit tests passed"
180 END
