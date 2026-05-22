/* memory.c — Region allocator implementation. */
#include "memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Region *region_create(size_t cap) {
    Region *r = malloc(sizeof(Region));
    r->base = malloc(cap);
    r->pos  = 0;
    r->cap  = cap;
    r->prev = NULL;
    return r;
}

void *region_alloc(Region *r, size_t size, size_t align) {
    size_t aligned = (r->pos + align - 1) & ~(align - 1);
    if (aligned + size > r->cap) {
        /* Grow: create a new region chunk */
        size_t new_cap = r->cap * 2 > aligned + size ? r->cap * 2 : aligned + size + 1024;
        Region *next = region_create(new_cap);
        next->prev = r;
        r = next;
        aligned = 0;
    }
    void *ptr = r->base + aligned;
    r->pos = aligned + size;
    return ptr;
}

void region_reset(Region *r) { if (r) r->pos = 0; }
void region_free(Region *r) {
    while (r) {
        Region *prev = r->prev;
        free(r->base);
        free(r);
        r = prev;
    }
}
