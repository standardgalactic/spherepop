#define _POSIX_C_SOURCE 200809L
/* evaluator.c — Event-driven evaluator.
 *
 * eval_node() is the recursive core, but it never calls provenance,
 * topology, or admissibility subsystems directly. It emits events and
 * those subsystems are listeners on the event bus.
 */
#include "evaluator.h"
#include "bubble.h"
#include "value.h"
#include "scope.h"
#include "history.h"
#include "provenance.h"
#include "admissibility.h"
#include "collapse.h"
#include "../parser/ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ---- Event bus ---- */

EventBus *event_bus_create(bool lazy) {
    EventBus *bus = calloc(1, sizeof(EventBus));
    bus->lazy = lazy;
    return bus;
}

void event_subscribe(EventBus *bus, EventType type,
                     EventListener listener, void *userdata) {
    int t = (int)type;
    if (bus->slots[t].n == bus->slots[t].cap) {
        bus->slots[t].cap = bus->slots[t].cap ? bus->slots[t].cap * 2 : 4;
        bus->slots[t].listeners = realloc(bus->slots[t].listeners,
            bus->slots[t].cap * sizeof(EventListener));
        bus->slots[t].userdata  = realloc(bus->slots[t].userdata,
            bus->slots[t].cap * sizeof(void *));
    }
    bus->slots[t].listeners[bus->slots[t].n] = listener;
    bus->slots[t].userdata[bus->slots[t].n]  = userdata;
    bus->slots[t].n++;
}

void event_emit(EventBus *bus, const SphereEvent *ev) {
    int t = (int)ev->type;
    for (int i = 0; i < bus->slots[t].n; i++)
        bus->slots[t].listeners[i](ev, bus->slots[t].userdata[i]);
}

/* ---- Standard listeners ---- */

void provenance_listener(const SphereEvent *ev, void *userdata) {
    EvalContext *ctx = (EvalContext *)userdata;
    if (ev->type != EV_POP_COMMIT) return;
    prov_record(ctx->prov, ev->bubble_id, PROV_ID_NULL,
                ctx->step, 1.0, 1.0, "pop", ev->payload);
}

void history_listener(const SphereEvent *ev, void *userdata) {
    EvalContext *ctx = (EvalContext *)userdata;
    if (ev->type == EV_POP_COMMIT)
        history_append(ctx->global_history, HE_POP_COMMIT,
                       ev->bubble_id, 1.0, 1.0, 1.0, ev->payload);
}

void admissibility_listener(const SphereEvent *ev, void *userdata) {
    (void)userdata;
    if (ev->type == EV_POP_BEGIN) {
        Bubble *b = (Bubble *)ev->payload;
        if (b) bubble_recalc_admissibility(b);
    }
}

void topology_listener(const SphereEvent *ev, void *userdata) {
    (void)userdata;
    /* Topology updates are already performed in bubble_pop();
     * this listener can drive visual export if a renderer is attached. */
    (void)ev;
}

/* ---- EvalContext ---- */

EvalContext *eval_context_create(bool trace) {
    EvalContext *ctx = calloc(1, sizeof(EvalContext));
    ctx->prov           = prov_store_create();
    ctx->global_history = history_create();
    ctx->bus            = event_bus_create(false);
    ctx->trace          = trace;

    /* Register standard listeners */
    event_subscribe(ctx->bus, EV_POP_BEGIN,  admissibility_listener, ctx);
    event_subscribe(ctx->bus, EV_POP_COMMIT, provenance_listener,    ctx);
    event_subscribe(ctx->bus, EV_POP_COMMIT, history_listener,       ctx);
    event_subscribe(ctx->bus, EV_POP_COMMIT, topology_listener,      ctx);
    return ctx;
}

void eval_context_free(EvalContext *ctx) {
    if (!ctx) return;
    prov_store_free(ctx->prov);
    history_free(ctx->global_history);
    free(ctx->bus);
    free(ctx);
}

/* ---- Builtins ---- */

static Value *builtin_sqrt(Value **args, int n) {
    if (n != 1) return val_error("sqrt: expected 1 argument");
    double x = args[0]->kind == VAL_INT ? (double)args[0]->u.ival
                                        : args[0]->u.fval;
    return val_float(sqrt(x));
}

static Value *builtin_abs(Value **args, int n) {
    if (n != 1) return val_error("abs: expected 1 argument");
    if (args[0]->kind == VAL_INT) return val_int(llabs(args[0]->u.ival));
    return val_float(fabs(args[0]->u.fval));
}

static Value *builtin_floor(Value **args, int n) {
    if (n != 1) return val_error("floor: expected 1 argument");
    double x = args[0]->kind == VAL_INT ? (double)args[0]->u.ival
                                        : args[0]->u.fval;
    return val_int((long long)floor(x));
}

static Value *builtin_ceil(Value **args, int n) {
    if (n != 1) return val_error("ceil: expected 1 argument");
    double x = args[0]->kind == VAL_INT ? (double)args[0]->u.ival
                                        : args[0]->u.fval;
    return val_int((long long)ceil(x));
}

static Value *builtin_len(Value **args, int n) {
    if (n != 1) return val_error("len: expected 1 argument");
    if (args[0]->kind == VAL_STRING)
        return val_int(strlen(args[0]->u.sval));
    if (args[0]->kind == VAL_LIST)
        return val_int(args[0]->u.list.n);
    return val_error("len: unsupported type");
}

static Value *builtin_str(Value **args, int n) {
    if (n != 1) return val_error("str: expected 1 argument");
    char buf[64];
    switch (args[0]->kind) {
        case VAL_INT:   snprintf(buf, sizeof(buf), "%lld", args[0]->u.ival); break;
        case VAL_FLOAT: snprintf(buf, sizeof(buf), "%g",   args[0]->u.fval); break;
        case VAL_BOOL:  snprintf(buf, sizeof(buf), "%s",   args[0]->u.bval ? "true" : "false"); break;
        case VAL_NULL:  snprintf(buf, sizeof(buf), "null"); break;
        case VAL_STRING: return val_string(args[0]->u.sval);
        default: snprintf(buf, sizeof(buf), "<value>"); break;
    }
    return val_string(buf);
}

static Value *builtin_int(Value **args, int n) {
    if (n != 1) return val_error("int: expected 1 argument");
    switch (args[0]->kind) {
        case VAL_INT:   return val_int(args[0]->u.ival);
        case VAL_FLOAT: return val_int((long long)args[0]->u.fval);
        case VAL_BOOL:  return val_int(args[0]->u.bval);
        case VAL_STRING: return val_int(atoll(args[0]->u.sval));
        default: return val_error("int: cannot convert");
    }
}

static Value *dispatch_builtin(const char *name, Value **args, int n) {
    if (strcmp(name, "sqrt")  == 0) return builtin_sqrt(args, n);
    if (strcmp(name, "abs")   == 0) return builtin_abs(args, n);
    if (strcmp(name, "floor") == 0) return builtin_floor(args, n);
    if (strcmp(name, "ceil")  == 0) return builtin_ceil(args, n);
    if (strcmp(name, "len")   == 0) return builtin_len(args, n);
    if (strcmp(name, "str")   == 0) return builtin_str(args, n);
    if (strcmp(name, "int")   == 0) return builtin_int(args, n);
    return NULL;
}

/* ---- Core evaluator ---- */

Value *eval_node(Node *node, Bubble *b, EvalContext *ctx) {
    if (!node) return val_null();

    switch (node->kind) {
        case NODE_INT_LIT:   return val_int(node->u.ival);
        case NODE_FLOAT_LIT: return val_float(node->u.fval);
        case NODE_STRING_LIT:return val_string(node->u.sval);
        case NODE_BOOL_LIT:  return val_bool(node->u.bval);
        case NODE_NULL_LIT:  return val_null();

        case NODE_IDENT: {
            Value *v = scope_lookup(b->scope, node->u.sval);
            if (!v) {
                char msg[128];
                snprintf(msg, sizeof(msg), "undefined: %s", node->u.sval);
                return val_error(msg);
            }
            val_retain(v);
            return v;
        }

        case NODE_BINOP: {
            Value *left  = eval_node(node->u.binop.left,  b, ctx);
            Value *right = eval_node(node->u.binop.right, b, ctx);
            const char *op = node->u.binop.op;
            Value *result;
            if      (strcmp(op, "+")  == 0) result = val_add(left, right);
            else if (strcmp(op, "-")  == 0) result = val_sub(left, right);
            else if (strcmp(op, "*")  == 0) result = val_mul(left, right);
            else if (strcmp(op, "/")  == 0) result = val_div(left, right);
            else if (strcmp(op, "%")  == 0) {
                if (left->kind == VAL_INT && right->kind == VAL_INT && right->u.ival != 0)
                    result = val_int(left->u.ival % right->u.ival);
                else result = val_error("modulo error");
            }
            else if (strcmp(op, "^")  == 0) {
                double lv = left->kind == VAL_INT ? (double)left->u.ival : left->u.fval;
                double rv = right->kind == VAL_INT ? (double)right->u.ival : right->u.fval;
                double pv = pow(lv, rv);
                /* Preserve integer type when both operands are non-negative integers */
                if (left->kind == VAL_INT && right->kind == VAL_INT && right->u.ival >= 0)
                    result = val_int((long long)pv);
                else
                    result = val_float(pv);
            }
            else if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
                     strcmp(op, "<")  == 0 || strcmp(op, ">")  == 0 ||
                     strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0)
                result = val_compare(left, right, op);
            else if (strcmp(op, "&&") == 0)
                result = val_bool(val_is_truthy(left) && val_is_truthy(right));
            else if (strcmp(op, "||") == 0)
                result = val_bool(val_is_truthy(left) || val_is_truthy(right));
            else { char msg[64]; snprintf(msg, sizeof(msg), "unknown op: %s", op);
                   result = val_error(msg); }
            val_release(left);
            val_release(right);
            return result;
        }

        case NODE_UNOP: {
            Value *operand = eval_node(node->u.unop.operand, b, ctx);
            const char *op = node->u.unop.op;
            Value *result;
            if      (strcmp(op, "-")  == 0) result = val_mul(val_int(-1), operand);
            else if (strcmp(op, "!")  == 0) result = val_bool(!val_is_truthy(operand));
            else { result = val_error("unknown unary op"); }
            val_release(operand);
            return result;
        }

        case NODE_LET: {
            Value *init = node->u.let.init
                            ? eval_node(node->u.let.init, b, ctx)
                            : val_null();
            scope_define(b->scope, node->u.let.name, init, node->u.let.mutable);
            val_release(init);
            return val_null();
        }

        case NODE_ASSIGN: {
            Value *v = eval_node(node->u.assign.value, b, ctx);
            if (!scope_assign(b->scope, node->u.assign.name, v)) {
                char msg[128];
                snprintf(msg, sizeof(msg), "cannot assign to %s (immutable or undefined)",
                         node->u.assign.name);
                val_release(v);
                return val_error(msg);
            }
            val_release(v);
            return val_null();
        }

        case NODE_BLOCK: {
            Value *last = val_null();
            for (int i = 0; i < node->u.block.n; i++) {
                val_release(last);
                last = eval_node(node->u.block.stmts[i], b, ctx);
                if (last && (last->kind == VAL_ERROR || last->kind == VAL_RETURN))
                    return last; /* Propagate return/error upward */
            }
            return last;
        }

        case NODE_IF: {
            Value *cond = eval_node(node->u.ifnode.cond, b, ctx);
            int truthy  = val_is_truthy(cond);
            val_release(cond);
            Value *branch_result;
            if (truthy)
                branch_result = eval_node(node->u.ifnode.then_branch, b, ctx);
            else if (node->u.ifnode.else_branch)
                branch_result = eval_node(node->u.ifnode.else_branch, b, ctx);
            else
                branch_result = val_null();
            return branch_result; /* propagates VAL_RETURN upward */
        }

        case NODE_WHILE: {
            Value *result = val_null();
            while (1) {
                Value *cond = eval_node(node->u.whilenode.cond, b, ctx);
                int truthy  = val_is_truthy(cond);
                val_release(cond);
                if (!truthy) break;
                val_release(result);
                result = eval_node(node->u.whilenode.body, b, ctx);
                if (result && result->kind == VAL_ERROR) return result;
            }
            return result;
        }

        case NODE_BUBBLE: {
            /* Create a nested admissibility region (§1.1) */
            Scope  *child_scope = scope_create(b->scope);
            Bubble *child       = bubble_alloc(b, child_scope);
            /* Evaluate all statements inside the bubble */
            Value *last = val_null();
            for (int i = 0; i < node->u.block.n; i++) {
                val_release(last);
                child->expression = node->u.block.stmts[i];
                last = eval_node(node->u.block.stmts[i], child, ctx);
                if (last && last->kind == VAL_ERROR) {
                    bubble_free(child);
                    scope_free(child_scope);
                    return last;
                }
            }
            /* Auto-pop the bubble: innermost-first evaluation */
            child->expression = node->u.block.n > 0
                                    ? node->u.block.stmts[node->u.block.n - 1]
                                    : NULL;
            Value *pop_result = bubble_pop(child, ctx);
            if (!pop_result) pop_result = last;
            scope_free(child_scope);
            return pop_result;
        }

        case NODE_POP: {
            Value *v = eval_node(node->u.child, b, ctx);
            /* pop() on a bubble ref pops it; on a plain value returns it */
            if (v && v->kind == VAL_BUBBLE_REF && v->u.bubble) {
                Value *r = bubble_pop(v->u.bubble, ctx);
                val_release(v);
                return r;
            }
            return v;
        }

        case NODE_REFUSE: {
            bubble_refuse(b, ctx);
            return val_null();
        }

        case NODE_PRINT: {
            Value *v = eval_node(node->u.child, b, ctx);
            val_print(v);
            printf("\n");
            val_release(v);
            return val_null();
        }

        case NODE_FN_DEF: {
            Value *fn = calloc(1, sizeof(Value));
            fn->kind  = VAL_FN;
            fn->ref_count = 1;
            fn->u.fn.name          = node->u.fn_def.name;
            fn->u.fn.params        = node->u.fn_def.params;
            fn->u.fn.n_params      = node->u.fn_def.n_params;
            fn->u.fn.body          = node->u.fn_def.body;
            fn->u.fn.closure_scope = b->scope;
            scope_define(b->scope, node->u.fn_def.name, fn, false);
            return val_null();
        }

        case NODE_FN_CALL: {
            /* Check builtins first */
            Value *args[64];
            int n_args = node->u.fn_call.n_args;
            for (int i = 0; i < n_args && i < 64; i++)
                args[i] = eval_node(node->u.fn_call.args[i], b, ctx);

            Value *bi = dispatch_builtin(node->u.fn_call.name, args, n_args);
            if (bi) {
                for (int i = 0; i < n_args; i++) val_release(args[i]);
                return bi;
            }

            /* User-defined function */
            Value *fn = scope_lookup(b->scope, node->u.fn_call.name);
            if (!fn || fn->kind != VAL_FN) {
                for (int i = 0; i < n_args; i++) val_release(args[i]);
                char msg[128];
                snprintf(msg, sizeof(msg), "undefined function: %s",
                         node->u.fn_call.name);
                return val_error(msg);
            }
            /* Retain fn across the recursive call so it isn't freed
             * mid-execution by a deeper scope teardown. */
            val_retain(fn);

            /* Build call scope. Use a lightweight bubble (no parent link)
             * to avoid entangling the parent's child list with recursive frames. */
            Scope  *call_scope = scope_create(fn->u.fn.closure_scope);
            Bubble *call_bubble = calloc(1, sizeof(Bubble));
            call_bubble->id           = bubble_next_id();
            call_bubble->state        = BUBBLE_ACTIVE;
            call_bubble->parent       = NULL; /* not in parent tree */
            call_bubble->scope        = call_scope;
            call_bubble->constraints  = cset_create();
            call_bubble->history      = history_create();
            call_bubble->prov_id      = PROV_ID_NULL;
            call_bubble->local_admissibility = 1.0;
            call_bubble->depth        = b->depth + 1;
            call_bubble->children_cap = 4;
            call_bubble->children     = malloc(4 * sizeof(Bubble *));

            for (int i = 0; i < fn->u.fn.n_params && i < n_args; i++)
                scope_define(call_scope, fn->u.fn.params[i], args[i], false);
            for (int i = 0; i < n_args; i++) val_release(args[i]);

            call_bubble->expression = fn->u.fn.body;
            Value *result = eval_node(fn->u.fn.body, call_bubble, ctx);

            /* Unwrap VAL_RETURN sentinel */
            if (result && result->kind == VAL_RETURN) {
                Value *inner = (Value *)result->u.bubble;
                free(result);
                result = inner;
            }

            /* Retain before scope teardown */
            if (result) val_retain(result);

            /* Leak call frame resources during recursion for now;
             * GC handles cleanup. This isolates the segfault location. */
            (void)call_bubble;
            val_release(fn);
            if (result) result->ref_count--;
            return result;
        }

        case NODE_FIELD_ACCESS: {
            Value *obj = eval_node(node->u.field_access.obj, b, ctx);
            if (!obj || obj->kind != VAL_BUBBLE_REF) {
                val_release(obj);
                return val_error("field access on non-bubble");
            }
            Value *field = scope_lookup(obj->u.bubble->scope,
                                        node->u.field_access.field);
            val_release(obj);
            if (!field) return val_null();
            val_retain(field);
            return field;
        }

        case NODE_RETURN: {
            Value *retval = eval_node(node->u.child, b, ctx);
            /* Wrap in VAL_RETURN sentinel so blocks propagate it */
            Value *sentinel = calloc(1, sizeof(Value));
            sentinel->kind = VAL_RETURN;
            sentinel->ref_count = 1;
            sentinel->u.bubble = (struct Bubble *)retval; /* reuse field for payload */
            return sentinel;
        }

        case NODE_OBSERVE: {
            Value *v = eval_node(node->u.child, b, ctx);
            /* observe() returns an admissibility point as a string summary */
            if (v && v->kind == VAL_BUBBLE_REF && v->u.bubble) {
                AdmissibilityPoint ap = admissibility_evaluate(v->u.bubble);
                char buf[128];
                snprintf(buf, sizeof(buf),
                         "<observe bubble=%llu local=%.3f action=%.3f strongly=%s>",
                         (unsigned long long)v->u.bubble->id,
                         ap.local, ap.action,
                         ap.strongly_admissible ? "yes" : "no");
                val_release(v);
                return val_string(buf);
            }
            return v;
        }

        case NODE_HISTORY: {
            /* Return global history summary as string */
            char buf[64];
            snprintf(buf, sizeof(buf),
                     "<history steps=%d action=%.3f>",
                     ctx->global_history->length,
                     ctx->global_history->total_action);
            return val_string(buf);
        }

        default: {
            char msg[64];
            snprintf(msg, sizeof(msg), "unimplemented node kind: %d", node->kind);
            return val_error(msg);
        }
    }
}

Value *eval_bubble(Bubble *b, EvalContext *ctx) {
    if (!b || !b->expression) return val_null();
    Value *result = eval_node(b->expression, b, ctx);
    b->result = result;
    return result;
}
