# Tokenizer Test Suite

This directory contains tests for the `basset_tokenize` debugging tool.

## Purpose

These tests verify that the tokenizer correctly breaks down BASIC source code into tokens. Each test consists of:
- A `.bas` file containing BASIC source code
- An `.expected` file containing the expected tokenization output

## Test Coverage

### test_basic.bas
Basic tokenization of a simple program with PRINT and END statements.

### test_operators.bas
Tests tokenization of all arithmetic, comparison, and logical operators:
- Arithmetic: `+`, `-`, `*`, `/`, `^`
- Comparison: `>`, `<`, `>=`, `<=`, `=`, `<>`
- Logical: `AND`, `OR`

### test_tab_function.bas
Tests the TAB function tokenization, verifying that `TAB(n)` is correctly recognized as a function token (0x79/121).

### test_functions.bas
Tests built-in function tokenization including:
- Math functions: `SIN`, `COS`, `ATN`, `INT`, `RND`
- String functions: `CHR$`, `STR$`

### test_control_flow.bas
Tests control flow statement tokenization:
- `FOR...TO...STEP`, `NEXT`
- `IF...THEN...GOTO`
- `GOSUB`

### test_strings.bas
Tests string variable and string comparison tokenization.

## Running Tests

Run all tokenizer tests:
```bash
./run.sh
```

Run all tests from project root:
```bash
make test              # All test suites
make test-tokenizer    # Just tokenizer tests
```

Or use make:
```bash
make test-tokenizer
```

## Adding New Tests

1. Create a new `.bas` file in this directory with your test case
2. Generate expected output:
   ```bash
   ./basset_tokenize tests/tokenizer/your_test.bas > tests/tokenizer/your_test.expected
   ```
3. Verify the expected output looks correct
4. Run tests to confirm: `./run_tokenizer_tests.sh`

## Test Output Format

Each test output shows:
```
Line  Col    Type          Name                  Value/Text
════════════════════════════════════════════════════════════
1     0      0x6B/107     NUMBER                10
1     3      0x20/32      PRINT                 
1     9      0x6C/108     STRING                "Hello"
```

Where:
- **Line**: Source line number (1-based)
- **Col**: Column position (0-based)
- **Type**: Token type in hex/decimal
- **Name**: Human-readable token name
- **Value/Text**: For numbers (numeric value) or strings/identifiers (text content)

## Debugging Failed Tests

If a test fails:

1. Run with verbose flag to see the diff:
   ```bash
   ./run_tokenizer_tests.sh -v
   ```

2. Manually inspect the output:
   ```bash
   ./basset_tokenize tests/tokenizer/failing_test.bas
   cat tests/tokenizer/failing_test.expected
   ```

3. If the new output is correct (e.g., after fixing a bug), regenerate:
   ```bash
   ./basset_tokenize tests/tokenizer/failing_test.bas > tests/tokenizer/failing_test.expected
   ```
