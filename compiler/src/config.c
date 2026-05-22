/* config.c — Runtime configuration and feature flags. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct SpConfig {
    bool   trace_events;
    bool   lazy_listeners;
    bool   print_history_on_exit;
    bool   verify_admissibility;
    double admissibility_threshold; /* Below this: refuse auto-triggered */
    int    max_bubble_depth;
    int    max_history_entries;
} SpConfig;

static SpConfig g_config = {
    .trace_events          = false,
    .lazy_listeners        = false,
    .print_history_on_exit = false,
    .verify_admissibility  = true,
    .admissibility_threshold = 0.0,
    .max_bubble_depth       = 256,
    .max_history_entries    = 65536
};

SpConfig *sp_config(void) { return &g_config; }

void sp_config_load_env(void) {
    const char *v;
    v = getenv("SP_TRACE");       if (v) g_config.trace_events = atoi(v) != 0;
    v = getenv("SP_LAZY");        if (v) g_config.lazy_listeners = atoi(v) != 0;
    v = getenv("SP_MAX_DEPTH");   if (v) g_config.max_bubble_depth = atoi(v);
    v = getenv("SP_ADM_THRESH");  if (v) g_config.admissibility_threshold = atof(v);
}

void sp_config_print(void) {
    printf("Spherepop Configuration:\n");
    printf("  trace_events:           %s\n", g_config.trace_events ? "on" : "off");
    printf("  lazy_listeners:         %s\n", g_config.lazy_listeners ? "on" : "off");
    printf("  verify_admissibility:   %s\n", g_config.verify_admissibility ? "on" : "off");
    printf("  admissibility_threshold: %.3f\n", g_config.admissibility_threshold);
    printf("  max_bubble_depth:       %d\n", g_config.max_bubble_depth);
    printf("  max_history_entries:    %d\n", g_config.max_history_entries);
}
