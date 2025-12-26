# Classic BASIC Test Programs

Here is a set of Classic BASIC test programs compatible with Atari BASIC and Microsoft BASIC, with:

* Suggested Filename
* Code (ready to type or save)
* Expected Output/Behavior

## BYE.BAS

Filename: `BYE.BAS`

```basic
BYE
```

Expected Output:
No output. The BASIC interpreter exits to the operating system or Memo Pad.

## END.BAS

Filename: `END.BAS`

```basic
10 PRINT "END TEST"
20 END
```

Expected Output:
Prints `END TEST` and ends the program.

## LET.BAS

Filename: `LET.BAS`

```basic
10 LET X=5
20 PRINT X
```

Expected Output:
Prints `5`

## REM.BAS

Filename: `REM.BAS`

```basic
10 REM THIS IS A COMMENT
20 PRINT "REM TEST"
```

Expected Output:
Prints `REM TEST`. The REM line is ignored by BASIC.

## STOP.BAS

Filename: `STOP.BAS`

```basic
10 PRINT "STOP TEST"
20 STOP
```

Expected Output:
Prints `STOP TEST`, then halts with `STOPPED AT LINE 20`.

## FORNEXT.BAS

Filename: `FORNEXT.BAS`

```basic
10 FOR I=1 TO 3 STEP 1
20 PRINT I
30 NEXT I
```

Expected Output:
Prints:

    1
    2
    3

## GOSUBRET.BAS

Filename: `GOSUBRET.BAS`

```basic
10 GOSUB 100
20 PRINT "BACK"
30 END
100 PRINT "IN SUBROUTINE"
110 RETURN
```

Expected Output:
Prints:

    IN SUBROUTINE
    BACK

## GOTO.BAS

Filename: `GOTO.BAS`

```basic
10 PRINT "GOTO TEST"
20 GOTO 40
30 PRINT "SHOULD NOT SEE THIS"
40 PRINT "GOTO WORKED"
```

Expected Output:
Prints:

    GOTO TEST
    GOTO WORKED

## IFTHEN.BAS

Filename: `IFTHEN.BAS`

```basic
10 X=5
20 IF X=5 THEN PRINT "IF/THEN WORKS"
```

Expected Output:
Prints:

    IF/THEN WORKS

## ONGOTO.BAS

Filename: `ONGOTO.BAS`

```basic
10 X=2
20 ON X GOTO 100,200,300
100 PRINT "ONE"
110 END
200 PRINT "TWO"
210 END
300 PRINT "THREE"
310 END
```

Expected Output:
Prints:

    TWO

## POP.BAS

Filename: `POP.BAS`

```basic
10 GOSUB 100
20 POP
30 PRINT "POP TEST"
40 END
100 RETURN
```

Expected Output:
Prints:

    POP TEST

## RESTORE.BAS

Filename: `RESTORE.BAS`

```basic
10 READ A
20 PRINT A
30 RESTORE
40 READ B
50 PRINT B
60 DATA 42
```

Expected Output:
Prints:

    42
    42

## TRAP.BAS

Filename: `TRAP.BAS`

```basic
10 TRAP 100
20 PRINT 1/0
30 END
100 PRINT "ERROR TRAPPED"
```

Expected Output:
Prints:

    ERROR TRAPPED

## ABS.BAS

Filename: `ABS.BAS`

```basic
10 PRINT ABS(-5)
```

Expected Output:
Prints:

    5

## CLOG.BAS

Filename: `CLOG.BAS`

```basic
10 PRINT CLOG(100)
```

Expected Output:
Prints:

    2

(Log base 10 of 100.)

## EXP.BAS

Filename: `EXP.BAS`

```basic
10 PRINT EXP(1)
```

Expected Output:
Prints:

    2.718282

(Approximate value of e^1.)

## INT.BAS

Filename: `INT.BAS`

```basic
10 PRINT INT(3.7)
```

Expected Output:
Prints:

    3

## LOG.BAS

Filename: `LOG.BAS`

```basic
10 PRINT LOG(10)
```

Expected Output:
Prints:

    2.302585

(Natural logarithm of 10.)

## RND.BAS

Filename: `RND.BAS`

```basic
10 PRINT RND(0)
```

Expected Output:
Prints a random number between 0 and 1 (never 1).
Example:

    0.54321

## SGN.BAS

Filename: `SGN.BAS`

```basic
10 PRINT SGN(-10)
```

Expected Output:
Prints:

    -1

## SQR.BAS

Filename: `SQR.BAS`

```basic
10 PRINT SQR(16)
```

Expected Output:
Prints:

    4

## ATN.BAS

Filename: `ATN.BAS`

```basic
10 PRINT ATN(1)
```

Expected Output:
Prints:

    0.785398

(Arctangent of 1 in radians.)

## COS.BAS

Filename: `COS.BAS`

```basic
10 PRINT COS(0)
```

Expected Output:
Prints:

    1

## SIN.BAS

Filename: `SIN.BAS`

```basic
10 PRINT SIN(0)
```

Expected Output:
Prints:

    0

## DEGRAD.BAS

Filename: `DEGRAD.BAS`

```basic
10 DEG
20 PRINT SIN(90)
30 RAD
40 PRINT SIN(3.14159/2)
```

Expected Output:
Prints:

    1
    1

(Sine of 90 degrees and π/2 radians.)

## ASC.BAS

Filename: `ASC.BAS`

```basic
10 PRINT ASC("A")
```

Expected Output:
Prints:

    65

(The ATASCII code for "A".)

## CHRS.BAS

Filename: `CHRS.BAS`

```basic
10 PRINT CHR$(65)
```

Expected Output:
Prints:

    A

## LEN.BAS

Filename: `LEN.BAS`

```basic
10 A$="TEST"
20 PRINT LEN(A$)
```

Expected Output:
Prints:

    4

## STRS.BAS

Filename: `STRS.BAS`

```basic
10 PRINT STR$(123)
```

Expected Output:
Prints:

    123

(The number 123 as a string.)

## VAL.BAS

Filename: `VAL.BAS`

```basic
10 PRINT VAL("456")
```

Expected Output:
Prints:

    456

(The string "456" converted to a number.)

## CONCAT.BAS

Filename: `CONCAT.BAS`

```basic
10 A$="HELLO":B$="WORLD"
20 C$=A$+B$
30 PRINT C$
```

Expected Output:
Prints:

    HELLOWORLD

## SPLITSTR.BAS

Filename: `SPLITSTR.BAS`

```basic
10 A$="ABCDEFG"
20 PRINT A$(2,4)
```

Expected Output:
Prints:

    BCD

(Characters 2 to 4 of "ABCDEFG".)

## DIM.BAS

Filename: `DIM.BAS`

```basic
10 DIM A(5)
20 FOR I=0 TO 5:A(I)=I:NEXT I
30 FOR I=0 TO 5:PRINT A(I);:NEXT I
```

Expected Output:
Prints:

    0 1 2 3 4 5

(All values separated by spaces on one line.)

## CLR.BAS

Filename: `CLR.BAS`

```basic
10 DIM A(5)
20 CLR
30 PRINT "CLEARED"
```

Expected Output:
Prints:

    CLEARED

(The array A is removed from memory.)

## POSITION.BAS

Filename: `POSITION.BAS`

```basic
10 POSITION 5,5
20 PRINT "HERE"
```

Expected Output:
Prints `HERE` at screen position (5,5).
(The word "HERE" appears at the specified location on the text screen.)

## ERRDIV0.BAS

Filename: `ERRDIV0.BAS`

```basic
10 TRAP 100
20 PRINT 1/0
30 END
100 PRINT "DIVISION BY ZERO ERROR CAUGHT"
```

Expected Output:
Prints:

    DIVISION BY ZERO ERROR CAUGHT

(The program traps the division by zero error and prints the message.)
