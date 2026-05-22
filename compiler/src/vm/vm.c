/* vm.c — Region VM execution engine (stub; see evaluator.c for tree-walker). */
#include "vm.h"
#include "../runtime/evaluator.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

VM *vm_create(struct EvalContext *ctx) {
    VM *v = calloc(1, sizeof(VM));
    v->ctx = ctx;
    return v;
}
void vm_free(VM *v) {
    if (!v) return;
    for (int i = 0; i < 64; i++) val_release(v->registers[i]);
    free(v);
}
void vm_load(VM *v, RegionInstruction *prog, int n) {
    v->program = prog;
    v->n_instructions = n;
    v->ip = 0;
}
Value *vm_run(VM *v) {
    /* Stub: full region VM execution to be implemented. */
    (void)v;
    return val_null();
}
void vm_dump(const VM *v) {
    printf("VM[ip=%d, n=%d]\n", v->ip, v->n_instructions);
    for (int i = 0; i < v->n_instructions; i++) {
        printf("  [%04d] %s region=%u\n", i,
               opcode_name(v->program[i].op),
               v->program[i].region);
    }
}
