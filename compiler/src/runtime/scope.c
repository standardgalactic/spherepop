#define _POSIX_C_SOURCE 200809L
#define _POSIX_C_SOURCE 200809L
/* scope.c — Scope as physical boundary of a bubble (§2.2). */
#include "scope.h"
#include "value.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Scope *scope_create(Scope *parent) {
    Scope *s = calloc(1, sizeof(Scope));
    s->parent = parent;
    s->depth  = parent ? parent->depth + 1 : 0;
    return s;
}

void scope_free(Scope *s) {
    if (!s) return;
    ScopeEntry *e = s->entries;
    while (e) {
        ScopeEntry *next = e->next;
        free(e->name);
        val_release(e->value);
        free(e);
        e = next;
    }
    free(s);
}

void scope_define(Scope *s, const char *name, Value *val, bool mutable) {
    /* Shadow any existing binding in this scope */
    ScopeEntry *e = s->entries;
    while (e) {
        if (strcmp(e->name, name) == 0) {
            val_release(e->value);
            val_retain(val);
            e->value   = val;
            e->mutable = mutable;
            return;
        }
        e = e->next;
    }
    ScopeEntry *ne = calloc(1, sizeof(ScopeEntry));
    ne->name    = strdup(name);
    ne->value   = val;
    ne->mutable = mutable;
    ne->next    = s->entries;
    s->entries  = ne;
    val_retain(val);
}

Value *scope_lookup(const Scope *s, const char *name) {
    const Scope *cur = s;
    while (cur) {
        ScopeEntry *e = cur->entries;
        while (e) {
            if (strcmp(e->name, name) == 0) return e->value;
            e = e->next;
        }
        cur = cur->parent;
    }
    return NULL;
}

bool scope_assign(Scope *s, const char *name, Value *val) {
    Scope *cur = s;
    while (cur) {
        ScopeEntry *e = cur->entries;
        while (e) {
            if (strcmp(e->name, name) == 0) {
                if (!e->mutable) return false;
                val_release(e->value);
                val_retain(val);
                e->value = val;
                return true;
            }
            e = e->next;
        }
        cur = cur->parent;
    }
    return false;
}

bool scope_contains_local(const Scope *s, const char *name) {
    ScopeEntry *e = s->entries;
    while (e) {
        if (strcmp(e->name, name) == 0) return true;
        e = e->next;
    }
    return false;
}

void scope_print(const Scope *s) {
    if (!s) { printf("<null scope>\n"); return; }
    printf("Scope[depth=%d]:\n", s->depth);
    ScopeEntry *e = s->entries;
    while (e) {
        printf("  %s = ", e->name);
        val_print(e->value);
        printf(" (%s)\n", e->mutable ? "mut" : "let");
        e = e->next;
    }
}
