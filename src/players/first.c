#include "chess.h"

static int eval(Board* b) {
    return 0;
}

static MoveSet ms = {};

static Move select_move(Board* b) {
    da_clear(&ms);
    int num_moves = legal_moves(b, &ms);
    if (num_moves == 0) {
        return NULL_MOVE;
    }
    return ms.at[0];
}

static void init() {
    if (ms.at == NULL) da_init(&ms, 64);
}

const Player player_first = {
    .name = "first",
    .init = init,
    .eval = eval,
    .select = select_move,
};