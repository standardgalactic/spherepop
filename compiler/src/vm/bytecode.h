/* bytecode.h — Bytecode emission from AST. */
#ifndef SP_BYTECODE_H
#define SP_BYTECODE_H
#include "opcode.h"
#include "../parser/ast.h"

typedef struct Bytecode {
    RegionInstruction *code;
    int                n;
    int                cap;
} Bytecode;

Bytecode *bytecode_create(void);
void      bytecode_free(Bytecode *bc);
void      bytecode_emit(Bytecode *bc, RegionInstruction instr);
void      bytecode_compile_node(Bytecode *bc, Node *node, RegionID region);

#endif
