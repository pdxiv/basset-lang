# Basset BASIC Test Suite

Comprehensive test suite for Basset BASIC with 145 tests covering table validation, functionality, error handling, and tokenization.

## Test Organization

```
tests/
├── run_all.sh        # Master test runner (runs all 4 test suites)
├── validate_tables.sh # Table coverage validation
├── standard/         # Functional tests (124 tests)
├── errors/           # Error detection tests (15 tests)
└── tokenizer/        # Tokenizer tests (6 tests)
```

## Running Tests

### Run All Tests
```bash
# From project root
make test

# Or directly
./tests/run_all.sh

# With verbose output
./tests/run_all.sh --verbose
```

### Run Individual Test Suites

**Standard Functional Tests:**
```bash
./tests/standard/run.sh
```

**Error Detection Tests:**
```bash
./tests/errors/run.sh
```

**Tokenizer Tests:**
```bash
./tests/tokenizer/run.sh
```

## Test Suite Details

### Standard Tests (121 tests)

Located in `standard/`, organized by category, these validate correct program execution:

**basics/** (12 tests)
- Variable assignment and expressions
- Arithmetic operations
- Simple programs

**control_flow/** (32 tests)
- IF/THEN/ELSE statements
- FOR/NEXT loops (including mismatch detection)
- GOTO/GOSUB/RETURN (including forward references)
- ON GOTO/GOSUB (including forward address tables)

**functions/** (7 tests)
- Mathematical functions (SIN, COS, ATN, EXP, LOG, SQR, ABS, INT, SGN)
- Utility functions (RND, PEEK)
- Comprehensive function tests

**strings/** (8 tests)
- String functions (LEN, VAL, STR$, ASC, CHR$, LEFT$, RIGHT$, MID$)
- String arrays and operations

**arrays/** (2 tests)
- Numeric arrays (1D and 2D)
- String arrays

**io/** (25 tests)
- PRINT statement variants
- INPUT statement
- File I/O (OPEN, CLOSE, PRINT#, INPUT#)
- Channel switching
- TAB function

**edge_cases/** (35 tests)
- Variable limits (128 numeric variables, 128 string variables)
- Array limits (64 arrays maximum)
- FOR loop nesting (32 levels maximum)
- GOSUB nesting (64 levels maximum)
- FOR/NEXT mismatch detection
- Complex statements
- Special cases
- Comments (REM and apostrophe)

**Test File Patterns:**
- `*.bas` - Test program source
- `*.bas.expected` - Expected output
- `*.bas.input` - Input data (for INPUT statements)
- `*.channel*.expected` - Expected file output from PRINT#

### Error Tests (14 tests)

Located in `errors/`, these validate proper error detection:

- Undefined line numbers (GOTO, GOSUB, ON)
- Syntax errors (missing operands, unbalanced parentheses)
- Duplicate line numbers
- Non-sequential line numbers
- Line numbers out of range
- Missing required tokens (equals signs)
- Unknown statements

**Test File Patterns:**
- `err_*.bas` - Program with error
- `err_*.bas.expected` - Expected error message

### Tokenizer Tests (6 tests)

Located in `tokenizer/`, these validate lexical analysis:

- Basic tokenization
- Operator recognition
- Function tokens
- Control flow keywords
- String handling
- TAB function (regression test)

**Test File Patterns:**
- `test_*.bas` - Program to tokenize
- `test_*.expected` - Expected token stream output

## Test Output

Tests generate temporary files during execution:
- `.out` files - Actual test output (in test directories)
- `.abc` files - Compiled bytecode (in /tmp and test directories)
- `channel*.txt` - File I/O output (in project root)
- `test_*.txt` - Additional test output files (in project root)
- `*.dat` - Test data files (in project root)

**Cleanup behavior:**
- ✅ Temporary files are automatically cleaned up after successful tests
- ✅ Failed tests preserve output files for debugging
- ✅ `make clean` removes all test artifacts

These files are ignored by git and won't clutter version control.

## Writing New Tests

### Standard Test
1. Create `tests/standard/test_name.bas`
2. Create `tests/standard/test_name.bas.expected` with expected output
3. If INPUT needed: create `tests/standard/test_name.bas.input`
4. If file I/O: create `tests/standard/test_name.channel*.expected`
5. Test runner auto-discovers new tests

### Error Test
1. Create `tests/errors/err_name.bas` with error condition
2. Create `tests/errors/err_name.bas.expected` with exact error message
3. Test must fail compilation with exit code 1

### Tokenizer Test
1. Create `tests/tokenizer/test_name.bas`
2. Run `./basset_tokenize tests/tokenizer/test_name.bas > tests/tokenizer/test_name.expected`
3. Review and verify expected output is correct

## Debugging Failed Tests

**View differences:**
```bash
SHOW_DIFF=1 ./tests/standard/run.sh
SHOW_DIFF=1 ./tests/errors/run.sh
VERBOSE=1 ./tests/tokenizer/run.sh
```

**Manual test:**
```bash
./basset_compile tests/standard/test_name.bas /tmp/test.abc
./basset_vm /tmp/test.abc
```

**Check tokenization:**
```bash
./basset_tokenize tests/standard/test_name.bas
```

## Test Coverage

Current coverage: **141 tests, 100% passing**

- ✅ All basic statements implemented
- ✅ All operators and precedence
- ✅ All built-in functions
- ✅ Control flow (IF/FOR/GOTO/GOSUB/ON)
- ✅ I/O operations (PRINT/INPUT/FILES)
- ✅ Arrays (1D/2D, numeric/string)
- ✅ Architectural limits (variables, arrays, nesting depth)
- ✅ Error detection
- ✅ Tokenizer correctness

## Integration with Build System

The Makefile provides convenient test targets:

```bash
make test          # Run all tests
make test-standard # Run standard tests only
make test-errors   # Run error tests only
make test-tokenizer # Run tokenizer tests only
```

Test failures cause non-zero exit codes for CI/CD integration.
