/* bytecode.c — Stub bytecode emitter. */
#include "bytecode.h"
#include <stdlib.h>
#include <string.h>

Bytecode *bytecode_create(void) {
    Bytecode *bc = calloc(1, sizeof(Bytecode));
    bc->cap  = 64;
    bc->code = malloc(bc->cap * sizeof(RegionInstruction));
    return bc;
}
void bytecode_free(Bytecode *bc) { if (bc) { free(bc->code); free(bc); } }

void bytecode_emit(Bytecode *bc, RegionInstruction instr) {
    if (bc->n == bc->cap) {
        bc->cap *= 2;
        bc->code = realloc(bc->code, bc->cap * sizeof(RegionInstruction));
    }
    bc->code[bc->n++] = instr;
}

void bytecode_compile_node(Bytecode *bc, Node *node, RegionID region) {
    if (!node || !bc) return;
    /* Stub: real implementation maps AST nodes to RegionInstructions. */
    RegionInstruction instr = {0};
    instr.region = region;
    instr.op     = OP_NOP;
    bytecode_emit(bc, instr);
}

const char *opcode_name(OpCode op) {
    switch (op) {
        case OP_NOP: return "NOP"; case OP_LOAD_INT: return "LOAD_INT";
        case OP_LOAD_FLOAT: return "LOAD_FLOAT"; case OP_LOAD_STRING: return "LOAD_STRING";
        case OP_LOAD_VAR: return "LOAD_VAR"; case OP_STORE_VAR: return "STORE_VAR";
        case OP_ADD: return "ADD"; case OP_SUB: return "SUB";
        case OP_MUL: return "MUL"; case OP_DIV: return "DIV";
        case OP_POP_BUBBLE: return "POP_BUBBLE"; case OP_REFUSE: return "REFUSE";
        case OP_COLLAPSE: return "COLLAPSE"; case OP_BIND: return "BIND";
        case OP_ENTER_REGION: return "ENTER_REGION"; case OP_EXIT_REGION: return "EXIT_REGION";
        case OP_CALL: return "CALL"; case OP_RETURN: return "RETURN";
        case OP_PRINT: return "PRINT"; case OP_HALT: return "HALT";
        default: return "?";
    }
}
