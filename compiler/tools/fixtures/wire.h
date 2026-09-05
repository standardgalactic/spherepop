#ifndef SP_FIXTURES_WIRE_H
#define SP_FIXTURES_WIRE_H

#include "kernel.h"

bool wire_encode(const Arbiter *arb, unsigned char **out, size_t *out_len, char *err);
bool wire_decode_replay(const unsigned char *data, size_t len, State *out, char *err);
void wire_fnv1a64_hex(const unsigned char *data, size_t len, char out[17]);

#endif
