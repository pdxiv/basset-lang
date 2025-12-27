10 REM Test TRAP with type mismatch errors
20 TRAP 200
30 PRINT "Test 1: LEN on number"
40 A = 42
50 B = LEN(A)
60 PRINT "ERROR: Should have trapped!"
70 END
200 REM Error handler
210 PRINT "TRAPPED: Type mismatch in LEN"
220 TRAP 300
230 PRINT "Test 2: String arithmetic"
240 A$ = "HELLO"
250 C = A$ + 5
260 PRINT "ERROR: Should have trapped!"
270 END
300 REM Second error handler
310 PRINT "TRAPPED: Type mismatch in arithmetic"
320 TRAP 400
330 PRINT "Test 3: ASC on number"
340 D = 65
350 E = ASC(D)
360 PRINT "ERROR: Should have trapped!"
370 END
400 REM Third error handler
410 PRINT "TRAPPED: Type mismatch in ASC"
420 PRINT "=== All TRAP type mismatch tests passed ==="
430 END
