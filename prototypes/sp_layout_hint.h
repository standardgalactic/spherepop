/* sp_layout_hint.h */
#ifndef SP_LAYOUT_HINT_H
#define SP_LAYOUT_HINT_H

#include <stdint.h>

#define SP_LAYOUT_VERSION 1

/* Flags describing semantic intent */
enum {
  SP_LH_SOFT        = 1 << 0, /* advisory only */
  SP_LH_PINNED      = 1 << 1, /* resist automatic layout */
  SP_LH_TEMPORAL    = 1 << 2, /* y encodes time / causal order */
  SP_LH_CLUSTER     = 1 << 3  /* intended cluster center */
};

typedef struct sp_layout_hint {
  uint32_t version;     /* SP_LAYOUT_VERSION */
  uint32_t flags;

  /* Position in semantic space (renderer chooses projection) */
  float x, y, z;

  /* Optional scale / salience */
  float radius;

  /* Optional orientation (for future use) */
  float nx, ny, nz;
} sp_layout_hint_t;

#endif /* SP_LAYOUT_HINT_H */