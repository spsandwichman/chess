#include "chess.h"

static int eval(Board* b) {
    return 0;
}

void print_board_w_moveset_w_start_square(Board* b, MoveSet* ms, u8 starting_square) {
    u8 highlights[64] = {};
    foreach (Move m, *ms) {
        if (m.start != starting_square) {
            continue;
        }
        highlights[m.target] += 1;
    }
    print_board(b, highlights);
}

static Move select_move(Board* b, int* eval_out) {
    static MoveSet ms = {};
    if (ms.at == NULL) da_init(&ms, 32);
    da_clear(&ms);

    legal_moves(b, &ms);
    if (ms.len == 0) {
        return NULL_MOVE;
    }
    printf("\n");

    start_again:

    print_board_w_moveset(b, &ms);

    char input_buf[100] = {};


    int start_index = -1;
    while (true) {

        printf("PLAYER :: start ");
        gets(input_buf);

        start_index = indexof(input_buf);

        if (strlen(input_buf) != 2) {
            start_index = -1;
        } else {
            bool is_valid_move_start = false;
            foreach (Move m, ms) {
                if (m.start == start_index) is_valid_move_start = true;
            }
            if (!is_valid_move_start) start_index = -1;
        }

        if (start_index == -1) {
            printf("invalid starting square.\n");
            continue;
        }

        print_board_w_moveset_w_start_square(b, &ms, start_index);
        break;
    }

    int target_index = -1;
    while (true) {

        printf("PLAYER :: target ");
        gets(input_buf);

        if (strcmp(input_buf, "back") == 0) {
            goto start_again;
        }

        target_index = indexof(input_buf);

        if (strlen(input_buf) != 2) {
            target_index = -1;
        } else {
            bool is_valid_move_start = false;
            foreach (Move m, ms) {
                if (m.start == start_index && m.target == target_index) is_valid_move_start = true;
            }
            if (!is_valid_move_start) target_index = -1;
        }

        if (target_index == -1) {
            printf("invalid target square.\n");
            continue;
        }
        break;
    }

    foreach (Move m, ms) {
        if (m.start == start_index && m.target == target_index) {
            return m;
        }
    }

    CRASH("somehow a legal move was half-detected (?) but could not be played");
    return NULL_MOVE;
}

static void init() {

}

const Player player_user = {
    .name = "user",
    .init = init,
    .eval = eval,
    .select = select_move,
};