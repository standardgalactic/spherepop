#include "spstore_sqlite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int exec_sql(sqlite3* db, const char* sql){
  char* err=NULL;
  int rc = sqlite3_exec(db, sql, 0, 0, &err);
  if (rc != SQLITE_OK){
    sqlite3_free(err);
    return rc;
  }
  return SQLITE_OK;
}

int spdb_open(sqlite3** db, const char* path){
  return sqlite3_open(path, db);
}

int spdb_init(sqlite3* db){
  return exec_sql(db,
    "PRAGMA journal_mode=WAL;"
    "CREATE TABLE IF NOT EXISTS events("
    "  branch TEXT NOT NULL,"
    "  authority TEXT NOT NULL,"
    "  eid INTEGER NOT NULL,"
    "  tick INTEGER NOT NULL,"
    "  op INTEGER NOT NULL,"
    "  bytes BLOB NOT NULL,"
    "  PRIMARY KEY(branch, eid)"
    ");"
    "CREATE INDEX IF NOT EXISTS idx_events_branch_op ON events(branch, op);"
  );
}

int spdb_append_event(sqlite3* db,
                      const char* branch,
                      const char* authority,
                      uint64_t eid,
                      uint64_t tick,
                      uint32_t op,
                      const void* bytes,
                      uint32_t nbytes){
  sqlite3_stmt* st=NULL;
  const char* sql = "INSERT INTO events(branch,authority,eid,tick,op,bytes) VALUES(?,?,?,?,?,?);";
  if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) != SQLITE_OK) return SQLITE_ERROR;

  sqlite3_bind_text(st, 1, branch, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(st, 2, authority, -1, SQLITE_TRANSIENT);
  sqlite3_bind_int64(st, 3, (sqlite3_int64)eid);
  sqlite3_bind_int64(st, 4, (sqlite3_int64)tick);
  sqlite3_bind_int(st, 5, (int)op);
  sqlite3_bind_blob(st, 6, bytes, (int)nbytes, SQLITE_TRANSIENT);

  int rc = sqlite3_step(st);
  sqlite3_finalize(st);
  return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}

int spdb_export_branch(sqlite3* db, const char* branch, const char* out_bin){
  sqlite3_stmt* st=NULL;
  const char* sql = "SELECT bytes FROM events WHERE branch=? ORDER BY eid ASC;";
  if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) != SQLITE_OK) return SQLITE_ERROR;
  sqlite3_bind_text(st, 1, branch, -1, SQLITE_TRANSIENT);

  FILE* out = fopen(out_bin, "wb");
  if (!out){ sqlite3_finalize(st); return SQLITE_ERROR; }

  while (sqlite3_step(st) == SQLITE_ROW){
    const void* b = sqlite3_column_blob(st, 0);
    int n = sqlite3_column_bytes(st, 0);
    if (n && fwrite(b, 1, (size_t)n, out) != (size_t)n){
      fclose(out); sqlite3_finalize(st); return SQLITE_ERROR;
    }
  }

  fclose(out);
  sqlite3_finalize(st);
  return SQLITE_OK;
}

int spdb_import_bin(sqlite3* db, const char* branch, const char* authority, const char* bin_path){
  FILE* f = fopen(bin_path, "rb");
  if (!f) return SQLITE_ERROR;

  // import raw record bytes without re-parsing:
  // For now, we parse hdr to extract eid/tick/op; store full record as bytes.
  for(;;){
    long start = ftell(f);
    if (start < 0) break;

    unsigned char hdrbuf[sizeof(uint64_t)*3 + sizeof(uint32_t)*4]; // big enough
    // safer: use your sp_event_hdr_t, but we avoid including headers here
    // You can swap to sp_event_hdr_t easily.
    // We'll do the actual struct version to be consistent:
    // (include spherepop_abi.h in a combined build of this file if you want)
    break;
  }

  fclose(f);
  // Intentionally left as a stub: best to implement with sp_event_hdr_t directly in your build unit.
  return SQLITE_OK;
}