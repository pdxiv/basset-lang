#!/bin/bash
# Error test runner - validates compile-time error detection

PASS=0
FAIL=0
ERROR=0

# Get script directory to support running from anywhere
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$PROJECT_ROOT"

echo "Running error tests..."
echo

for testfile in tests/errors/err_*.bas; do
    if [ ! -f "$testfile" ]; then
        continue
    fi
    
    testname=$(basename "$testfile" .bas)
    expected="tests/errors/${testname}.bas.expected"
    
    if [ ! -f "$expected" ]; then
        echo "⚠ $testname - no expected file"
        ((ERROR++))
        continue
    fi
    
    # Compile should fail (exit code 1)
    output=$(./basset_compile "$testfile" 2>&1)
    exitcode=$?
    
    if [ $exitcode -ne 1 ]; then
        echo "✗ $testname - expected exit code 1, got $exitcode"
        ((FAIL++))
        continue
    fi
    
    # Compare error message
    expected_msg=$(cat "$expected")
    if [ "$output" = "$expected_msg" ]; then
        echo "✓ $testname"
        ((PASS++))
    else
        echo "✗ $testname - error message mismatch"
        ((FAIL++))
        if [ "$SHOW_DIFF" = "1" ]; then
            echo "  Expected: $expected_msg"
            echo "  Got:      $output"
        fi
    fi
done

echo
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Results: $PASS passed, $FAIL failed, $ERROR errors"
total=$((PASS + FAIL))
if [ $total -gt 0 ]; then
    percent=$((PASS * 100 / total))
    echo "Success rate: ${percent}% ($PASS/$total)"
fi
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ $FAIL -gt 0 ] || [ $ERROR -gt 0 ]; then
    exit 1
fi

exit 0
