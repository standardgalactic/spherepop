/* sp_replay_json.c */
#include "sp_replay.h"
#include <stdio.h>

/* Example: POP handler */
static sp_status_t on_pop_json(sp_replay_ctx_t* ctx,
                               const sp_event_hdr_t* hdr,
                               const void* payload) {
  const sp_evt_pop_t* e = (const sp_evt_pop_t*)payload;

  printf("{\"eid\":%llu,\"op\":\"POP\",\"obj\":%llu,\"parent\":%llu,\"objtype\":%u}",
         (unsigned long long)hdr->eid,
         (unsigned long long)e->p.a,
         (unsigned long long)e->p.b,
         e->objtype);
  printf("\n");
  return SP_OK;
}

/* Stub handlers for others */
static sp_status_t on_link_json(sp_replay_ctx_t* ctx,
                                const sp_event_hdr_t* hdr,
                                const void* payload) {
  const sp_evt_link_t* e = (const sp_evt_link_t*)payload;
  printf("{\"eid\":%llu,\"op\":\"LINK\",\"src\":%llu,\"dst\":%llu}\n",
         (unsigned long long)hdr->eid,
         (unsigned long long)e->p.a,
         (unsigned long long)e->p.b);
  return SP_OK;
}

/* Build handler table */
void sp_replay_json_handlers(sp_replay_handlers_t* h) {
  *h = (sp_replay_handlers_t){
    .on_pop  = on_pop_json,
    .on_link = on_link_json,
  };
}