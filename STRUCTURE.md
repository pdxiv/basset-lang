# Project Structure

This document describes the organization of the BASIC implementation.

## Directory Layout

```
.
├── basset_compile.c       # Compiler executable (BASIC → bytecode)
├── basset_vm.c            # Virtual machine executable
├── basset_disasm.c        # Disassembler executable
├── basset_asm.c           # Assembler executable
├── basset_tokenize.c      # Tokenizer debugger executable
│
├── src/                   # Core library modules
│   ├── tokenizer.c/h      # Lexical analysis
│   ├── tokens.h           # Token definitions
│   ├── parser.c/h         # Syntax analysis
│   ├── syntax_tables.c/h  # Grammar tables
│   ├── compiler.c/h       # Code generation
│   ├── bytecode.h         # VM instruction set
│   ├── vm.c/h             # Bytecode interpreter
│   ├── bytecode_file.c/h  # File I/O (.abc format)
│   ├── floating_point.c/h # Numeric operations
│   └── README.md          # Module documentation
│
├── tests/                 # Test suite
│   ├── README.md          # Test documentation
│   ├── run_all.sh         # Master test runner
│   ├── standard/          # Standard functional tests (110 tests)
│   │   ├── run.sh         # Standard test runner
│   │   ├── *.bas          # Test programs
│   │   ├── *.bas.expected # Expected outputs
│   │   └── *.bas.input    # Input data for interactive tests
│   ├── errors/            # Compile-time error tests (14 tests)
│   │   ├── run.sh         # Error test runner
│   │   ├── README.md      # Error test documentation
│   │   └── err_*.bas      # Error test programs
│   └── tokenizer/         # Tokenizer-specific tests (6 tests)
│       ├── run.sh         # Tokenizer test runner
│       ├── README.md      # Tokenizer test documentation
│       └── test_*.bas     # Tokenizer tests
│
├── docs/                  # Reference documentation
│   ├── Implementation_Overview.md
│   ├── Token_Reference.md
│   ├── Bytecode_Reference.md
│   ├── Syntax_Tables_Reference.md
│   ├── VM_Architecture.md
│   └── README.md
│
├── obj/                   # Build artifacts (generated)
│
├── Makefile               # Build system
├── .gitignore             # Git exclusions
├── README.md              # Project documentation
├── QUICKSTART.md          # Quick start guide
└── STRUCTURE.md           # This file
```

## File Organization Principles

### Executables at Root
All main program entry points are at the project root:
- Easy to find
- Clear separation from library code
- Consistent naming: `basset_<function>.c`

### Library Code in src/
Reusable modules that executables link against:
- Implementation separated from interface (.c vs .h)
- Grouped by compilation pipeline phase
- Each module has single responsibility

### Tests Self-Contained
Each test suite is in its own directory with its runner:
- `tests/standard/` - Functional tests (110 tests) with run.sh
- `tests/errors/` - Error detection (14 tests) with run.sh
- `tests/tokenizer/` - Tokenizer tests (6 tests) with run.sh
- `tests/run_all.sh` - Master runner for all suites

### Documentation Organized
Reference materials and implementation documentation:
- `docs/` - Complete implementation documentation
- Root `.md` files - Project documentation (README, QUICKSTART, STRUCTURE)

## Build Artifacts

Generated during build (not in version control):
- `obj/*.o` - Object files
- `basset_compile` - Compiler binary
- `basset_vm` - VM binary
- `basset_disasm` - Disassembler binary
- `basset_asm` - Assembler binary
- `basset_tokenize` - Tokenizer debugger binary
- `tests/*.abc` - Compiled bytecode (test artifacts)
- `*.txt` - Test output files

## Module Dependencies

```
Compilation Pipeline:
  Source (.bas)
    ↓
  tokenizer → tokens
    ↓
  parser + syntax_tables → AST
    ↓
  compiler → bytecode
    ↓
  bytecode_file → .abc file

Execution Pipeline:
  .abc file
    ↓
  bytecode_file → bytecode in memory
    ↓
  vm + floating_point → execution
```

## Adding New Files

**New executable:**
- Place `.c` file at root
- Add build rule in Makefile (see existing patterns)
- Add to `make all` target

**New library module:**
- Place `.c` and `.h` in `src/`
- Add to appropriate `*_SOURCES` variable in Makefile
- Document in `src/README.md`
appropriate `tests/` subdirectory
- Create `.bas.expected` with expected output
- Test runner will automatically discover it

Run tests:
```bash
make test                    # All tests
./tests/run_all.sh          # Same as make test
./tests/standard/run.sh     # Just functional tests
./tests/errors/run.sh       # Just error tests
./tests/tokenizer/run.sh    # Just tokenizer tests
```
- Create `.bas.expected` with expected output
- Test runner will automatically discover it

## Design Rationale

**Why flat src/ directory?**
- Currently 11 .c files - still easy to navigate
- Subdirectories add complexity without clear benefit yet
- Will reorganize into subdirectories if/when src/ exceeds ~15-20 files

**Why separate test directories?**
- Different test types have different purposes
- Easier to run subsets (e.g., only tokenizer tests)
- Clear organization as test count grows (130 tests currently)

**Why docs/ directory?**
- Separates reference materials from project documentation
- Keeps root clean with only essential project docs
- Clear distinction between "how to use" vs "how it was built"
