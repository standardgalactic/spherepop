/* history.h — Computation history: ordered sequence of pop events.
 *
 * History is the primary ontological object in Spherepop. A computation
 * is a history, not a mapping from input to output. The terminal value
 * is a feature of the history. See Monograph §3.1, §12.1, §26.4.
 */
#ifndef SP_HISTORY_H
#define SP_HISTORY_H

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t BubbleID;
typedef uint64_t Timestamp;

typedef enum {
    HE_POP_BEGIN,
    HE_POP_COMMIT,
    HE_REFUSE,
    HE_COLLAPSE,
    HE_BIND,
    HE_SCOPE_ENTER,
    HE_SCOPE_EXIT,
    HE_CONSTRAINT_UPDATE,
    HE_ADMISSIBILITY_RECALC
} HistoryEventKind;

typedef struct HistoryEntry {
    HistoryEventKind kind;
    BubbleID         bubble_id;
    Timestamp        t;
    double           admissibility_before;
    double           admissibility_after;
    double           action_delta;
    void            *payload;
    struct HistoryEntry *next;
} HistoryEntry;

typedef struct History {
    HistoryEntry *head;
    HistoryEntry *tail;
    int           length;
    double        total_action;   /* S[γ] = Σ action_delta */
} History;

History *history_create(void);
void     history_free(History *h);
void     history_append(History *h, HistoryEventKind kind, BubbleID bid,
                        double adm_before, double adm_after,
                        double action_delta, void *payload);
void     history_print(const History *h);
History *history_merge(const History *a, const History *b);
double   history_action(const History *h);
bool     history_is_admissible_trajectory(const History *h);
bool     history_equivalent(const History *a, const History *b,
                             int collapse_level);

#endif /* SP_HISTORY_H */
