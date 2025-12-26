# Error Test Suite

This directory contains test cases for error detection and reporting in Basset BASIC.

## Test Categories

### Line Number Validation (3 tests)
- **err_line_too_large.bas** - Line number exceeds 32767
- **err_non_sequential.bas** - Line numbers not in sequential order
- **err_duplicate_line.bas** - Duplicate line numbers

### Syntax Errors (7 tests)
- **err_missing_equals.bas** - LET statement without =
- **err_implied_let_missing_equals.bas** - Implied LET without =
- **err_missing_operand.bas** - Expression missing operand
- **err_unbalanced_parens.bas** - Unclosed parenthesis
- **err_no_line_number.bas** - Statement without line number
- **err_unknown_statement.bas** - Invalid statement keyword
- **err_multiple_errors.bas** - Multiple errors in one file

### Semantic Errors (4 tests)
- **err_undefined_goto.bas** - GOTO to non-existent line
- **err_undefined_gosub.bas** - GOSUB to non-existent line
- **err_undefined_on.bas** - ON statement with undefined targets
- **err_multiple_undefined.bas** - Multiple undefined line references

## Test Format

Each test consists of two files:
- `err_*.bas` - Source file with error
- `err_*.bas.expected` - Expected error output

## Running Tests

Run error tests:
```bash
./run.sh
```

Run all tests:
```bash
make test              # From project root
../run_all.sh          # From tests/ directory
```

## Expected Results

All error tests should:
1. Exit with code 1 (compilation error)
2. Produce error output matching the .expected file
3. Include line number and error description

## Adding New Tests

1. Create `tests/errors/err_newtest.bas` with error case
2. Create `tests/errors/err_newtest.bas.expected` with expected output
3. Run `./run_error_tests.sh` to verify

Example expected output format:
```
ERROR at line 20: <error message>
  <source line>
  <spaces>^

Compilation failed with N error(s)
```
