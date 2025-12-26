#!/bin/bash

# Tokenizer test runner
# Tests that basset_tokenize produces consistent output

PASS=0
FAIL=0
ERRORS=0
TOTAL=0

# Get script directory to support running from anywhere
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$PROJECT_ROOT"

echo "╔═════════════════════════════════════════════════════╗"
echo "║        BASIC Tokenizer Test Suite Runner            ║"
echo "╚═════════════════════════════════════════════════════╝"
echo ""

# Check if basset_tokenize exists
if [ ! -f "./basset_tokenize" ]; then
    echo "Error: basset_tokenize not found. Run 'make' first."
    exit 1
fi

for test_file in tests/tokenizer/*.bas; do
    # Skip if no .bas files found
    [ -e "$test_file" ] || continue
    
    base_name="${test_file%.bas}"
    expected_file="${base_name}.expected"
    
    # Skip if no expected output file exists
    if [ ! -f "$expected_file" ]; then
        echo "⚠ $(basename "$test_file") - no expected file"
        continue
    fi
    
    TOTAL=$((TOTAL + 1))
    test_name=$(basename "$test_file" .bas)
    
    # Run tokenizer
    output_file="${base_name}.out"
    ./basset_tokenize "$test_file" > "$output_file" 2>&1
    exit_code=$?
    
    if [ $exit_code -ne 0 ]; then
        echo "✗ $test_name - TOKENIZER ERROR (exit code $exit_code)"
        ERRORS=$((ERRORS + 1))
        continue
    fi
    
    # Compare output with expected
    if diff -q "$output_file" "$expected_file" > /dev/null 2>&1; then
        echo "✓ $test_name"
        PASS=$((PASS + 1))
        rm -f "$output_file"  # Clean up on success
    else
        echo "✗ $test_name - OUTPUT MISMATCH"
        FAIL=$((FAIL + 1))
        if [ "${VERBOSE}" = "1" ]; then
            echo "  Expected output in: $expected_file"
            echo "  Actual output in: $output_file"
            echo "  Diff:"
            diff -u "$expected_file" "$output_file" | head -20 | sed 's/^/    /'
            echo ""
        fi
    fi
done

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Results: $PASS passed, $FAIL failed, $ERRORS errors"
if [ $TOTAL -gt 0 ]; then
    PERCENT=$((PASS * 100 / TOTAL))
    echo "Success rate: ${PERCENT}% ($PASS/$TOTAL)"
fi
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ $FAIL -gt 0 ] || [ $ERRORS -gt 0 ]; then
    echo ""
    echo "To see differences for failed tests, run:"
    echo "  VERBOSE=1 $0"
    exit 1
fi

exit 0
