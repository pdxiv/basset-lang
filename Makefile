.DEFAULT_GOAL := all

CC = gcc
CFLAGS = -Wall -O2 -ansi -pedantic
LDFLAGS = -lm

# Primary binaries (production workflow)
COMPILER = basset_compile
VM = basset_vm
DISASM = basset_disasm
ASM = basset_asm
TOKENIZE = basset_tokenize

SRCDIR = src
TOOLSDIR = tools
OBJDIR = obj

# Common source files
COMMON_SOURCES = $(SRCDIR)/tokenizer.c \
                 $(SRCDIR)/parser.c \
                 $(SRCDIR)/syntax_tables.c \
                 $(SRCDIR)/floating_point.c

COMPILER_SOURCES = $(SRCDIR)/compiler.c \
                   $(SRCDIR)/bytecode_file.c

VM_SOURCES = $(SRCDIR)/vm.c

# Primary binary sources
COMPILE_SOURCES = basset_compile.c \
                  $(COMMON_SOURCES) \
                  $(COMPILER_SOURCES)

VM_STANDALONE_SOURCES = basset_vm.c \
                        $(SRCDIR)/vm.c \
                        $(SRCDIR)/bytecode_file.c \
                        $(SRCDIR)/floating_point.c

DISASM_SOURCES = basset_disasm.c \
                 $(SRCDIR)/bytecode_file.c \
                 $(SRCDIR)/floating_point.c

ASM_SOURCES = basset_asm.c \
              $(SRCDIR)/bytecode_file.c \
              $(SRCDIR)/floating_point.c

# Object files
COMPILE_OBJECTS = $(OBJDIR)/basset_compile.o \
                  $(OBJDIR)/tokenizer.o \
                  $(OBJDIR)/parser.o \
                  $(OBJDIR)/compiler.o \
                  $(OBJDIR)/bytecode_file.o \
                  $(OBJDIR)/syntax_tables.o \
                  $(OBJDIR)/floating_point.o

VM_OBJECTS = $(OBJDIR)/basset_vm.o \
             $(OBJDIR)/vm.o \
             $(OBJDIR)/bytecode_file.o \
             $(OBJDIR)/compiler.o \
             $(OBJDIR)/parser.o \
             $(OBJDIR)/tokenizer.o \
             $(OBJDIR)/syntax_tables.o \
             $(OBJDIR)/floating_point.o

DISASM_OBJECTS = $(OBJDIR)/basset_disasm.o \
                 $(OBJDIR)/bytecode_file.o \
                 $(OBJDIR)/compiler.o \
                 $(OBJDIR)/parser.o \
                 $(OBJDIR)/tokenizer.o \
                 $(OBJDIR)/syntax_tables.o \
                 $(OBJDIR)/floating_point.o

ASM_OBJECTS = $(OBJDIR)/basset_asm.o \
              $(OBJDIR)/bytecode_file.o \
              $(OBJDIR)/compiler.o \
              $(OBJDIR)/parser.o \
              $(OBJDIR)/tokenizer.o \
              $(OBJDIR)/syntax_tables.o \
              $(OBJDIR)/floating_point.o

TOKENIZE_OBJECTS = $(OBJDIR)/basset_tokenize.o \
                   $(OBJDIR)/tokenizer.o \
                   $(OBJDIR)/syntax_tables.o \
                   $(OBJDIR)/floating_point.o \
                   $(OBJDIR)/parser.o

# Help target
help:
	@echo "╔═════════════════════════════════════════════════════╗"
	@echo "║        BASIC Compiler - Available Targets           ║"
	@echo "╚═════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Build Targets:"
	@echo "  make               Build all binaries (compiler, VM, disassembler, assembler)"
	@echo "  make all           Same as 'make'"
	@echo "  make clean         Remove all build artifacts and binaries"
	@echo ""
	@echo "Test Targets:"
	@echo "  make test              Run all test suites (validation + standard + error + tokenizer)"
	@echo "  make check             Alias for 'make test'"
	@echo "  make test-validation   Validate table-driven architecture coverage"
	@echo "  make test-standard     Run standard test suite (121 tests)"
	@echo "  make test-errors       Run error test suite (14 tests)"
	@echo "  make test-tokenizer    Run tokenizer test suite (6 tests)"
	@echo ""
	@echo "Individual Binaries:"
	@echo "  make basset_compile   Build the BASIC compiler"
	@echo "  make basset_vm        Build the bytecode VM"
	@echo "  make basset_disasm    Build the disassembler"
	@echo "  make basset_asm       Build the assembler"
	@echo "  make basset_tokenize  Build the tokenizer debugger"
	@echo ""
	@echo "Usage Examples:"
	@echo "  make && make test         Build everything and run all tests"
	@echo "  ./basset_compile prog.bas Compile a BASIC program"
	@echo "  ./basset_vm prog.abc      Run compiled bytecode"
	@echo ""

all: $(COMPILER) $(VM) $(DISASM) $(ASM) $(TOKENIZE)

$(COMPILER): $(COMPILE_OBJECTS)
	$(CC) $(COMPILE_OBJECTS) $(LDFLAGS) -o $(COMPILER)

$(VM): $(VM_OBJECTS)
	$(CC) $(VM_OBJECTS) $(LDFLAGS) -o $(VM)

$(DISASM): $(DISASM_OBJECTS)
	$(CC) $(DISASM_OBJECTS) $(LDFLAGS) -o $(DISASM)

$(ASM): $(ASM_OBJECTS)
	$(CC) $(ASM_OBJECTS) $(LDFLAGS) -o $(ASM)

$(TOKENIZE): $(TOKENIZE_OBJECTS)
	$(CC) $(TOKENIZE_OBJECTS) $(LDFLAGS) -o $(TOKENIZE)

# Build rules for main program files
$(OBJDIR)/basset_compile.o: basset_compile.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -c $< -o $@

$(OBJDIR)/basset_vm.o: basset_vm.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -c $< -o $@

$(OBJDIR)/basset_disasm.o: basset_disasm.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -c $< -o $@

$(OBJDIR)/basset_asm.o: basset_asm.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -c $< -o $@

$(OBJDIR)/basset_tokenize.o: basset_tokenize.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -c $< -o $@

# Build rules for src files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Directory creation
$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(COMPILER) $(VM) $(DISASM) $(ASM) $(TOKENIZE)
	rm -f tests/standard/*.abc tests/standard/*.out /tmp/test_*.abc
	rm -f tests/tokenizer/*.out
	rm -f channel*.txt test_*.txt *.dat

# Test targets
test: all test-validation test-standard test-errors test-tokenizer
	@echo
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo "All test suites completed successfully!"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

test-clean:
	@rm -f tests/standard/*.abc tests/standard/*.out /tmp/test_*.abc
	@rm -f tests/tokenizer/*.out
	@rm -f channel*.txt test_*.txt

test-validation:
	@echo "Running table validation..."
	@./tests/validate_tables.sh

test-standard: all test-clean
	@echo "Running standard test suite..."
	@./tests/standard/run.sh

test-errors: all
	@echo "Running error test suite..."
	@./tests/errors/run.sh

test-tokenizer: all
	@echo "Running tokenizer test suite..."
	@./tests/tokenizer/run.sh

check: test

.PHONY: all clean test test-clean test-validation test-standard test-errors test-tokenizer check help
