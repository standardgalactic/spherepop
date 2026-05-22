/* opcode.h — RegionInstruction opcode set.
 *
 * The Spherepop VM uses RegionInstructions, not stack instructions.
 * Each instruction carries region, constraint, and provenance context.
 * ConstraintRef indexes into a region-scoped constraint table rather
 * than using a flat bitmask. See design discussion in ARCHITECTURE.md.
 */
#ifndef SP_OPCODE_H
#define SP_OPCODE_H

#include <stdint.h>

typedef uint32_t RegionID;
typedef uint32_t ConstraintRef; /* Index into region constraint table */
typedef uint32_t ProvenanceRef;
typedef uint32_t NodeRef;

typedef enum {
    OP_NOP,
    OP_LOAD_INT,
    OP_LOAD_FLOAT,
    OP_LOAD_STRING,
    OP_LOAD_BOOL,
    OP_LOAD_NULL,
    OP_LOAD_VAR,
    OP_STORE_VAR,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_POW,
    OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LEQ, OP_GEQ,
    OP_AND, OP_OR, OP_NOT, OP_NEG,
    OP_POP_BUBBLE,   /* Explicit pop event — not just "compute and discard" */
    OP_REFUSE,       /* Mark region inadmissible */
    OP_COLLAPSE,     /* Apply collapse operator */
    OP_BIND,         /* Establish constraint membrane */
    OP_ENTER_REGION, /* Open a new admissibility region */
    OP_EXIT_REGION,  /* Close a region */
    OP_CALL,
    OP_RETURN,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_PRINT,
    OP_OBSERVE,
    OP_HISTORY,
    OP_HALT
} OpCode;

/* A region instruction carries topology alongside the operation.
 * ConstraintRef is a table index, not a bitmask — this preserves
 * relational constraint structure. See design notes, §ARCHITECTURE. */
typedef struct RegionInstruction {
    RegionID      region;
    ConstraintRef constraints; /* Index into region's constraint table */
    ProvenanceRef provenance;
    OpCode        op;
    NodeRef       operands[4];
    int64_t       imm;         /* Immediate integer value */
    double        fimm;        /* Immediate float value */
    const char   *simm;        /* Immediate string value */
} RegionInstruction;

const char *opcode_name(OpCode op);

#endif /* SP_OPCODE_H */
