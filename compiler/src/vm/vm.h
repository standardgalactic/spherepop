/* vm.h — Region VM interface. */
#ifndef SP_VM_H
#define SP_VM_H
#include "opcode.h"
#include "../runtime/value.h"

typedef struct VM {
    RegionInstruction *program;
    int                n_instructions;
    int                ip;            /* Instruction pointer */
    struct Value      *registers[64]; /* Register file */
    int                sp;            /* Stack pointer (for nested calls) */
    struct EvalContext *ctx;
} VM;

VM           *vm_create(struct EvalContext *ctx);
void          vm_free(VM *v);
void          vm_load(VM *v, RegionInstruction *prog, int n);
struct Value *vm_run(VM *v);
void          vm_dump(const VM *v);

#endif
