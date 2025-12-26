#!/bin/bash

# Master test runner - runs all test suites
# Usage: ./tests/run_all.sh [--verbose]

set -e  # Exit on first failure

# Get script directory to support running from anywhere
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

# Parse arguments
VERBOSE=0
if [ "$1" = "--verbose" ] || [ "$1" = "-v" ]; then
    VERBOSE=1
    export SHOW_DIFF=1
fi

echo "╔════════════════════════════════════════════════════════════╗"
echo "║          BASIC - Master Test Runner                 ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Check that binaries exist
if [ ! -f "./basset_compile" ] || [ ! -f "./basset_vm" ]; then
    echo "Error: Binaries not found. Run 'make' first."
    exit 1
fi

echo "═══════════════════════════════════════════════════════════════"
echo " Standard Functional Tests"
echo "═══════════════════════════════════════════════════════════════"
echo ""
./tests/standard/run.sh
STANDARD_EXIT=$?

echo ""
echo "═══════════════════════════════════════════════════════════════"
echo " Error Detection Tests"
echo "═══════════════════════════════════════════════════════════════"
echo ""
./tests/errors/run.sh
ERRORS_EXIT=$?

echo ""
echo "═══════════════════════════════════════════════════════════════"
echo " Tokenizer Tests"
echo "═══════════════════════════════════════════════════════════════"
echo ""
./tests/tokenizer/run.sh
TOKENIZER_EXIT=$?

echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║                    OVERALL RESULTS                         ║"
echo "╚════════════════════════════════════════════════════════════╝"

if [ $STANDARD_EXIT -eq 0 ]; then
    echo "✓ Standard tests: PASS"
else
    echo "✗ Standard tests: FAIL"
fi

if [ $ERRORS_EXIT -eq 0 ]; then
    echo "✓ Error tests: PASS"
else
    echo "✗ Error tests: FAIL"
fi

if [ $TOKENIZER_EXIT -eq 0 ]; then
    echo "✓ Tokenizer tests: PASS"
else
    echo "✗ Tokenizer tests: FAIL"
fi

echo ""

if [ $STANDARD_EXIT -eq 0 ] && [ $ERRORS_EXIT -eq 0 ] && [ $TOKENIZER_EXIT -eq 0 ]; then
    echo "All test suites completed successfully!"
    exit 0
else
    echo "Some test suites failed."
    if [ $VERBOSE -eq 0 ]; then
        echo "Run with --verbose flag to see details."
    fi
    exit 1
fi
