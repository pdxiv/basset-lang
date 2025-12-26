#!/bin/bash

# Standard functional test runner for Classic BASIC compiler
PASS=0
FAIL=0
ERRORS=0
TOTAL=0

# Get script directory to support running from anywhere
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$PROJECT_ROOT"

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║          Classic BASIC Test Suite Runner                  ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

# Find all .bas files recursively in subdirectories
while IFS= read -r test_file; do
    base_name="${test_file%.bas}"
    expected_file="${base_name}.bas.expected"
    input_file="${base_name}.bas.input"
    
    # Skip if no expected output file exists
    if [ ! -f "$expected_file" ]; then
        continue
    fi
    
    TOTAL=$((TOTAL + 1))
    test_name=$(basename "$test_file" .bas)
    
    # Compile the BASIC file
    ./basset_compile "$test_file" "/tmp/${test_name}.abc" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "✗ $test_name - COMPILE ERROR"
        ERRORS=$((ERRORS + 1))
        continue
    fi
    
    # Run the compiled bytecode
    # If .input file exists, pipe it to the VM for INPUT statements
    if [ -f "$input_file" ]; then
        ./basset_vm "/tmp/${test_name}.abc" < "$input_file" > "${base_name}.out" 2>&1
    else
        ./basset_vm "/tmp/${test_name}.abc" > "${base_name}.out" 2>&1
    fi
    exit_code=$?
    
    # Compare output with expected
    output_matches=true
    if ! diff -q "${base_name}.out" "$expected_file" > /dev/null 2>&1; then
        output_matches=false
    fi
    
    # Check for file channel output tests (files created by PRINT#)
    file_checks_pass=true
    for channel_expected in "${base_name}".channel*.expected; do
        [ -e "$channel_expected" ] || continue
        
        # Extract the filename pattern from the expected file
        # e.g., test_foo.channel1.expected -> channel1.txt
        channel_num=$(echo "$channel_expected" | sed 's/.*\.channel\([0-9]\)\.expected/\1/')
        actual_file="channel${channel_num}.txt"
        
        if [ ! -f "$actual_file" ]; then
            file_checks_pass=false
            if [ "${SHOW_DIFF}" = "1" ]; then
                echo "  Missing file: $actual_file"
            fi
            break
        fi
        
        if ! diff -q "$actual_file" "$channel_expected" > /dev/null 2>&1; then
            file_checks_pass=false
            if [ "${SHOW_DIFF}" = "1" ]; then
                echo "  File mismatch: $actual_file vs $channel_expected"
                diff -u "$channel_expected" "$actual_file" | head -10 | sed 's/^/    /'
            fi
            break
        fi
    done
    
    if $output_matches && $file_checks_pass; then
        echo "✓ $test_name"
        PASS=$((PASS + 1))
        # Clean up temporary files on success
        rm -f "${base_name}.out"
        rm -f channel*.txt test_*.txt *.dat
    else
        echo "✗ $test_name - OUTPUT MISMATCH"
        FAIL=$((FAIL + 1))
        if [ "${SHOW_DIFF}" = "1" ]; then
            if ! $output_matches; then
                echo "  Expected output in: $expected_file"
                echo "  Actual output in: ${base_name}.out"
                echo "  First 10 lines of diff:"
                diff -u "$expected_file" "${base_name}.out" | head -15 | sed 's/^/    /'
            fi
            echo ""
        fi
    fi
done < <(find tests/standard -type f -name "*.bas" | sort)

# Final cleanup of any remaining temporary files
rm -f channel*.txt test_*.txt *.dat

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
    echo "  SHOW_DIFF=1 $0"
    exit 1
fi

exit 0
