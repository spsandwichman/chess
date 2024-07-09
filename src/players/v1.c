#include "chess.h"

// v1 - evaluate the board after every possible move and choose the move that gives the best evaluation

static int eval(Board* b) {
    int diff_pawns = 0;
    int diff_rooks = 0;
    int diff_knights = 0;
    int diff_bishops = 0;
    int diff_queens = 0;
    int diff_kings = 0;

    for_range(i, 0, 64) {
        u8 p = b->board[i];
        int offset = piece_color(p) == b->color_to_move ? 1 : -1;
        switch (piece_type(p)) {
        case PAWN:   diff_pawns   += offset; break;
        case ROOK:   diff_rooks   += offset; break;
        case KNIGHT: diff_knights += offset; break;
        case BISHOP: diff_bishops += offset; break;
        case QUEEN:  diff_queens  += offset; break;
        case KING:   diff_kings   += offset; break;
        default:
            break;
        }
    }

    int diff_mobility = pseudo_legal_moves(b, NULL, false);
    swap_color_to_move(*b);
    diff_mobility -= pseudo_legal_moves(b, NULL, false);
    swap_color_to_move(*b);

    return 
        2000*(diff_kings) +
        9*(diff_queens) +
        5*(diff_rooks) +
        3*(diff_bishops + diff_knights) +
        1*(diff_pawns) +
        1*(diff_mobility);
}

static MoveSet ms = {};

static Move select_move(Board* b) {
    da_clear(&ms);
    int num_moves = legal_moves(b, &ms);
    if (num_moves == 0) {
        return NULL_MOVE;
    }

    // printf("color %s\n", b->color_to_move == WHITE ? "white" : "black");
    
    Move best = ms.at[0];
    make_move(b, best, false);
    int best_eval = eval(b);
    undo_move(b, false);

    for_range(i, 1, ms.len) {
        Move m = ms.at[i];

        make_move(b, m, false);
        int m_eval = eval(b);
        undo_move(b, false);

        if (m_eval > best_eval) {
            best_eval = m_eval;
            best = m;
        }
    }

    // printf("color %s\n", b->color_to_move == WHITE ? "white" : "black");

    return best;
}

static void init() {
    if (ms.at == NULL) da_init(&ms, 64);
}

const Player player_v1 = {
    .name = "v1",
    .init = init,
    .eval = eval,
    .select = select_move,
};