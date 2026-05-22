#define _POSIX_C_SOURCE 200809L
#define _POSIX_C_SOURCE 200809L
/* value.c — Value allocation, arithmetic, and reference counting. */
#include "value.h"
#include "bubble.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static Value *val_alloc(ValueKind k) {
    Value *v = calloc(1, sizeof(Value));
    if (!v) { perror("val_alloc"); exit(1); }
    v->kind = k;
    v->ref_count = 1;
    return v;
}

Value *val_int(long long i)   { Value *v = val_alloc(VAL_INT);    v->u.ival = i; return v; }
Value *val_float(double f)    { Value *v = val_alloc(VAL_FLOAT);  v->u.fval = f; return v; }
Value *val_bool(int b)        { Value *v = val_alloc(VAL_BOOL);   v->u.bval = !!b; return v; }
Value *val_null(void)         { return val_alloc(VAL_NULL); }

Value *val_string(const char *s) {
    Value *v = val_alloc(VAL_STRING);
    v->u.sval = strdup(s);
    return v;
}

Value *val_bubble_ref(struct Bubble *b) {
    Value *v = val_alloc(VAL_BUBBLE_REF);
    v->u.bubble = b;
    return v;
}

Value *val_error(const char *msg) {
    Value *v = val_alloc(VAL_ERROR);
    v->u.error_msg = strdup(msg);
    return v;
}

void val_retain(Value *v)  { if (v) v->ref_count++; }
void val_release(Value *v) {
    if (!v) return;
    if (--v->ref_count <= 0) {
        if (v->kind == VAL_STRING)     free(v->u.sval);
        if (v->kind == VAL_ERROR)      free(v->u.error_msg);
        if (v->kind == VAL_LIST) {
            for (int i = 0; i < v->u.list.n; i++)
                val_release(v->u.list.items[i]);
            free(v->u.list.items);
        }
        free(v);
    }
}

void val_print(const Value *v) {
    if (!v) { printf("<null>"); return; }
    switch (v->kind) {
        case VAL_INT:        printf("%lld", v->u.ival); break;
        case VAL_FLOAT:      printf("%g", v->u.fval); break;
        case VAL_STRING:     printf("%s", v->u.sval); break;
        case VAL_BOOL:       printf("%s", v->u.bval ? "true" : "false"); break;
        case VAL_NULL:       printf("null"); break;
        case VAL_BUBBLE_REF: printf("<bubble:%llu>",
                                    v->u.bubble ?
                                    (unsigned long long)v->u.bubble->id : 0); break;
        case VAL_ERROR:      printf("<error:%s>", v->u.error_msg); break;
        default:             printf("<value:%d>", v->kind);
    }
}

const char *val_type_name(ValueKind k) {
    switch (k) {
        case VAL_INT:        return "int";
        case VAL_FLOAT:      return "float";
        case VAL_STRING:     return "string";
        case VAL_BOOL:       return "bool";
        case VAL_NULL:       return "null";
        case VAL_BUBBLE_REF: return "bubble";
        case VAL_HISTORY_REF:return "history";
        case VAL_FN:         return "fn";
        case VAL_LIST:       return "list";
        case VAL_ERROR:      return "error";
    }
    return "unknown";
}

int val_is_truthy(const Value *v) {
    if (!v) return 0;
    switch (v->kind) {
        case VAL_BOOL:   return v->u.bval;
        case VAL_INT:    return v->u.ival != 0;
        case VAL_FLOAT:  return v->u.fval != 0.0;
        case VAL_STRING: return v->u.sval && v->u.sval[0] != '\0';
        case VAL_NULL:   return 0;
        default:         return 1;
    }
}

Value *val_add(const Value *a, const Value *b) {
    if (!a || !b) return val_error("null operand in add");
    if (a->kind == VAL_INT && b->kind == VAL_INT)
        return val_int(a->u.ival + b->u.ival);
    if ((a->kind == VAL_FLOAT || a->kind == VAL_INT) &&
        (b->kind == VAL_FLOAT || b->kind == VAL_INT)) {
        double av = a->kind == VAL_FLOAT ? a->u.fval : (double)a->u.ival;
        double bv = b->kind == VAL_FLOAT ? b->u.fval : (double)b->u.ival;
        return val_float(av + bv);
    }
    if (a->kind == VAL_STRING && b->kind == VAL_STRING) {
        size_t la = strlen(a->u.sval), lb = strlen(b->u.sval);
        char *s = malloc(la + lb + 1);
        memcpy(s, a->u.sval, la);
        memcpy(s + la, b->u.sval, lb);
        s[la + lb] = '\0';
        Value *r = val_string(s);
        free(s);
        return r;
    }
    return val_error("type error in +");
}

Value *val_sub(const Value *a, const Value *b) {
    if (!a || !b) return val_error("null operand in sub");
    if (a->kind == VAL_INT && b->kind == VAL_INT)
        return val_int(a->u.ival - b->u.ival);
    double av = a->kind == VAL_FLOAT ? a->u.fval : (double)a->u.ival;
    double bv = b->kind == VAL_FLOAT ? b->u.fval : (double)b->u.ival;
    return val_float(av - bv);
}

Value *val_mul(const Value *a, const Value *b) {
    if (!a || !b) return val_error("null operand in mul");
    if (a->kind == VAL_INT && b->kind == VAL_INT)
        return val_int(a->u.ival * b->u.ival);
    double av = a->kind == VAL_FLOAT ? a->u.fval : (double)a->u.ival;
    double bv = b->kind == VAL_FLOAT ? b->u.fval : (double)b->u.ival;
    return val_float(av * bv);
}

Value *val_div(const Value *a, const Value *b) {
    if (!a || !b) return val_error("null operand in div");
    double bv = b->kind == VAL_FLOAT ? b->u.fval : (double)b->u.ival;
    if (bv == 0.0) return val_error("division by zero");
    if (a->kind == VAL_INT && b->kind == VAL_INT && b->u.ival != 0)
        return val_int(a->u.ival / b->u.ival);
    double av = a->kind == VAL_FLOAT ? a->u.fval : (double)a->u.ival;
    return val_float(av / bv);
}

Value *val_compare(const Value *a, const Value *b, const char *op) {
    if (!a || !b) return val_error("null in compare");
    double av, bv;
    if (a->kind == VAL_STRING && b->kind == VAL_STRING) {
        int cmp = strcmp(a->u.sval, b->u.sval);
        if (strcmp(op, "==") == 0) return val_bool(cmp == 0);
        if (strcmp(op, "!=") == 0) return val_bool(cmp != 0);
        if (strcmp(op, "<")  == 0) return val_bool(cmp <  0);
        if (strcmp(op, ">")  == 0) return val_bool(cmp >  0);
        return val_error("unsupported string comparison");
    }
    av = a->kind == VAL_FLOAT ? a->u.fval : (double)a->u.ival;
    bv = b->kind == VAL_FLOAT ? b->u.fval : (double)b->u.ival;
    if (strcmp(op, "==") == 0) return val_bool(av == bv);
    if (strcmp(op, "!=") == 0) return val_bool(av != bv);
    if (strcmp(op, "<")  == 0) return val_bool(av <  bv);
    if (strcmp(op, ">")  == 0) return val_bool(av >  bv);
    if (strcmp(op, "<=") == 0) return val_bool(av <= bv);
    if (strcmp(op, ">=") == 0) return val_bool(av >= bv);
    return val_error("unknown comparison operator");
}
