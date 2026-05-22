/* memory.h — Region allocator for nested bubble lifetimes.
 *
 * Region allocation is natural for bubbles: a bubble and all its
 * descendants share a region lifetime that ends at pop. This beats
 * per-object malloc for deeply nested computations.
 */
#ifndef SP_MEMORY_H
#define SP_MEMORY_H
#include <stddef.h>

typedef struct Region {
    char *base;
    size_t pos;
    size_t cap;
    struct Region *prev;
} Region;

Region *region_create(size_t cap);
void   *region_alloc(Region *r, size_t size, size_t align);
void    region_free(Region *r);
void    region_reset(Region *r);

#endif
