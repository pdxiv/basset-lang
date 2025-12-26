/* basset_vm.c - Standalone VM for executing bytecode files */
#include <stdio.h>
#include <stdlib.h>
#include "vm.h"
#include "bytecode_file.h"

int main(int argc, char **argv) {
    CompiledProgram *prog;
    VMState *vm;
    
    /* Check arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program.abc>\n", argv[0]);
        fprintf(stderr, "  Executes compiled BASIC bytecode\n");
        return 1;
    }
    
    /* Load bytecode file */
    prog = bytecode_file_load(argv[1]);
    if (!prog) {
        fprintf(stderr, "Failed to load bytecode file\n");
        return 1;
    }
    
    /* Initialize VM */
    vm = vm_init(prog);
    if (!vm) {
        fprintf(stderr, "VM initialization failed\n");
        compiled_program_free(prog);
        return 1;
    }
    
    /* Execute */
    vm_execute(vm);
    
    /* Cleanup */
    vm_free(vm);
    compiled_program_free(prog);
    
    return 0;
}
