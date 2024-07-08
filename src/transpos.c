#include "chess.h"

void ttable_init(TransposTable* tt, u64 len) {
    tt->len = len;
    tt->at = malloc(sizeof(TransposEntry) * tt->len);
    assert(tt->at != NULL);
    memset(tt->at, 0, sizeof(TransposEntry) * tt->len);
}

TransposEntry* ttable_get(TransposTable* tt, u64 zobrist, int depth, int alpha, int beta) {
    TransposEntry* e = &tt->at[zobrist % tt->len];
    if (e->zobrist != zobrist) return NULL;

    if (e->depth >= depth) {
        // use information about the eval to figure out 
        // if we should keep searching or not
        if (e->kind == TT_UPPER && e->eval <= alpha) return e;
        if (e->kind == TT_LOWER && e->eval > beta)   return e;
    }

    return NULL;
}

void ttable_put(TransposTable* tt, u64 zobrist, int eval, u16 depth, u8 kind) {
    tt->at[zobrist % tt->len] = (TransposEntry) {
        .zobrist = zobrist,
        .eval = eval,
        .depth = depth,
        .kind = kind,
    };
}