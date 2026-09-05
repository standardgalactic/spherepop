/* json.h — Minimal dependency-free JSON parser for the flat fixture
 * runner. Mirrors the shape (and limitations) of spherepop-kernel's
 * src/json.rs: just enough JSON to read experiments/flat/fixtures files,
 * not a general-purpose JSON library.
 */
#ifndef SP_FIXTURES_JSON_H
#define SP_FIXTURES_JSON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonKind;

typedef struct Json {
    JsonKind kind;
    union {
        bool    boolean;
        double  number;
        char   *string;
        struct { struct Json **items; size_t len; }               array;
        struct { char **keys; struct Json **values; size_t len; } object;
    } u;
} Json;

/* Parses `text` into a freshly-allocated Json tree. On failure, returns
 * NULL and writes a human-readable message into *err (caller must free
 * neither on failure; *err is a static/allocated string owned by the
 * parser and freed by json_free_error if non-NULL). */
Json *json_parse(const char *text, char **err);
void  json_free_error(char *err);
void  json_free(Json *v);

/* Accessors. All return NULL / default if the field is absent or the
 * wrong kind -- callers that require a field to be present should check
 * for NULL and fail loudly (these fixtures are trusted well-formed
 * inputs, so a NULL here indicates a fixture bug worth surfacing). */
const Json *json_get(const Json *obj, const char *key);          /* object field, or NULL */
bool         json_get_bool(const Json *obj, const char *key, bool dflt);
const char  *json_get_str(const Json *obj, const char *key, const char *dflt);
bool         json_as_u64(const Json *v, uint64_t *out);          /* NUMBER -> u64 */
const char  *json_as_str(const Json *v);                         /* STRING -> chars, else NULL */
bool         json_as_bool(const Json *v, bool dflt);
size_t       json_array_len(const Json *v);                      /* 0 if not an array */
const Json  *json_array_get(const Json *v, size_t i);

#endif /* SP_FIXTURES_JSON_H */
