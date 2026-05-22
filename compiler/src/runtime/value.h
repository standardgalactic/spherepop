/* value.h — Runtime value types. */
#ifndef SP_VALUE_H
#define SP_VALUE_H

#include <stdint.h>

typedef enum {
    VAL_INT, VAL_FLOAT, VAL_STRING, VAL_BOOL, VAL_NULL,
    VAL_BUBBLE_REF,  /* Reference to a live bubble */
    VAL_HISTORY_REF, /* Reference to a history object */
    VAL_FN,          /* Closure/function */
    VAL_LIST,
    VAL_ERROR,
    VAL_RETURN  /* Sentinel: wraps a return value for propagation */
} ValueKind;

struct Bubble;
struct History;

typedef struct Value {
    ValueKind kind;
    int       ref_count;
    union {
        long long   ival;
        double      fval;
        char       *sval;
        int         bval;
        struct Bubble  *bubble;
        struct History *history;
        struct {
            char        *name;
            char       **params;
            int          n_params;
            struct Node *body;
            struct Scope *closure_scope;
        } fn;
        struct {
            struct Value **items;
            int            n;
            int            cap;
        } list;
        char *error_msg;
    } u;
} Value;

Value *val_int(long long i);
Value *val_float(double f);
Value *val_string(const char *s);
Value *val_bool(int b);
Value *val_null(void);
Value *val_bubble_ref(struct Bubble *b);
Value *val_error(const char *msg);

void        val_retain(Value *v);
void        val_release(Value *v);
void        val_print(const Value *v);
const char *val_type_name(ValueKind k);
int         val_is_truthy(const Value *v);
Value      *val_add(const Value *a, const Value *b);
Value      *val_sub(const Value *a, const Value *b);
Value      *val_mul(const Value *a, const Value *b);
Value      *val_div(const Value *a, const Value *b);
Value      *val_compare(const Value *a, const Value *b, const char *op);

#endif /* SP_VALUE_H */
