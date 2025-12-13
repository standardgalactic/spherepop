#ifndef SPSTORE_SQLITE_H
#define SPSTORE_SQLITE_H

#include <stdint.h>
#include <sqlite3.h>

int spdb_open(sqlite3** db, const char* path);
int spdb_init(sqlite3* db);

int spdb_append_event(sqlite3* db,
                      const char* branch,
                      const char* authority,
                      uint64_t eid,
                      uint64_t tick,
                      uint32_t op,
                      const void* bytes,
                      uint32_t nbytes);

int spdb_export_branch(sqlite3* db, const char* branch, const char* out_bin);
int spdb_import_bin(sqlite3* db, const char* branch, const char* authority, const char* bin_path);

#endif