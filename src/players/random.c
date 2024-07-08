#include "chess.h"

static int eval(Board* b) {
    return 0;
}

static MoveSet ms = {0};

static Move select_move(Board* b) {
    da_clear(&ms);

    int num_moves = legal_moves(b, &ms);
    if (num_moves == 0) {
        return NULL_MOVE;
    }
    if (num_moves == 1) {
        return ms.at[0];
    }
    return ms.at[rand() % num_moves];
}

static void init() {
    struct timeval time = {};
    gettimeofday(&time, NULL);
    srand((int)time.tv_usec);

    if (ms.at == NULL) da_init(&ms, 64);
}

const Player player_random = {
    .name = "random",
    .init = init,
    .eval = eval,
    .select = select_move,
};