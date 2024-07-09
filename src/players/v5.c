#include "chess.h"

#define SEARCH_BUFFER_LEN 256
#define QSEARCH_BUFFER_LEN 256

// #define FUCKING_HATE_STALEMATES

static TransposTable tt = {};

static int piece_value(u8 kind) {
    switch (kind) {
    case KING: return 2000;
    case QUEEN: return 9;
    case ROOK: return 5;
    case BISHOP: return 4;
    case KNIGHT: return 3;
    case PAWN: return 1;
    }
    return 0;
};

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

static void order_moves(Board* b, MoveSet* ms) {
    int* scores = alloca(ms->len * sizeof(int));
    
    // assign scores
    for_range(i, 0, ms->len) {
        Move m = ms->at[i];

        int score = 0;
        u8 moved_type = piece_type(b->board[m.start]);
        u8 captured_type = piece_type(b->board[m.target]);

        if (captured_type != EMPTY) score += piece_value(captured_type) - piece_value(moved_type);

        if (moved_type == PAWN) switch (m.special) {
        case SPECIAL_PROMOTE_QUEEN:  score += piece_value(QUEEN); break;
        case SPECIAL_PROMOTE_KNIGHT: score += piece_value(KNIGHT); break;
        case SPECIAL_PROMOTE_ROOK:   score += piece_value(ROOK); break;
        case SPECIAL_PROMOTE_BISHOP: score += piece_value(BISHOP); break;
        }

        scores[i] = score;
    }

    for_range(i, 1, ms->len) {
        for (int j = i; j > 0 && scores[j-1] < scores[j]; j--) {
            int s = scores[j];
            scores[j] = scores[j-1];
            scores[j-1] = s;

            Move m = ms->at[j];
            ms->at[j] = ms->at[j-1];
            ms->at[j-1] = m;
        }
    }
}

static int q_search(Board* b, int alpha, int beta) {
    int evaluation = eval(b);
    if (evaluation >= beta) return beta;
    alpha = max(alpha, evaluation);

    MoveSet ms = {};
    // da_init(&ms, 80);

    // RISKY BUSINESS FOR SURE
    // BAD SANDWICH BAD SANDWICH BAD SANDWICH
    Move backing_buffer[128];
    ms.at = backing_buffer;
    ms.cap = 128;
    ms.len = 0;

    legal_captures(b, &ms);
    order_moves(b, &ms);
    for_range(i, 0, ms.len) {
        Move m = ms.at[i];

        make_move(b, m, true);
        evaluation = -q_search(b, -beta, -alpha);
        undo_move(b, true);

        if (evaluation >= beta) return beta;
        alpha = max(alpha, evaluation);
    }

    // da_destroy(&ms);
    return alpha;
}

static Move best_move;
static int  best_eval;
static int  search_depth;

static int search(Board* b, int depth, int alpha, int beta) {
    
    if (depth == 0) { // bottom
        return q_search(b, alpha, beta);
    }

    if (depth < search_depth) {
        // avoid stalemate by repetition
        if (history_contains(b, b->zobrist)) {
#ifdef FUCKING_HATE_STALEMATES
        return -100000;
#else
        return 0;
#endif
        }
    }

    TransposEntry* entry = ttable_get(&tt, b->zobrist, depth, alpha, beta);
    if (entry != NULL && depth != search_depth) return entry->eval; // dont accept stored evaluations on the root


    u8 bound = TT_UPPER;
    
    MoveSet moveset = {};
    Move backing_buffer[256];

    moveset.cap = 256;
    // BAD BAD BAD BAD
    moveset.at = backing_buffer;
    moveset.len = 0;

    MoveSet* ms = &moveset;

    legal_moves(b, ms);
    // int x = pseudo_legal_moves(b, ms, false);
    // printf("LEGAL: %d\n", ms->len);

    // exit(0);

    if (ms->len == 0) {
        // test if we're in check
        bool in_check;
        swap_color_to_move(*b);
        pseudo_legal_moves(b, ms, true);
        swap_color_to_move(*b);
        foreach (Move m, *ms) {
            if (piece_type(b->board[m.target]) == KING) {
                in_check = true;
                break;
            }
        }

        if (in_check) {
            return -100000; // checkmated
#ifndef FUCKING_HATE_STALEMATES
        } else {
            return 0; // stalemated
#endif
        }
    }

    order_moves(b, ms);


    foreach (Move m, *ms) {

        make_move(b, m, true);
        int evaluation = -search(b, depth - 1, -beta, -alpha);
        undo_move(b, true);

        if (evaluation >= beta) {
            ttable_put(&tt, b->zobrist, beta, depth, TT_LOWER);
            return beta;
        }

        // found a new best move!
        if (evaluation > alpha) {
            alpha = evaluation;
            if (depth == search_depth) {
                // printf("select move %s -> %s\n",
                    // square_names[m.start],
                    // square_names[m.target]);
                best_move = m;
                best_eval = evaluation;
            }

            bound = TT_EXACT;
        }
    }

    ttable_put(&tt, b->zobrist, alpha, depth, bound);
    return alpha;
}

static Move select_move(Board* b) {

    best_eval = -200000;
    best_move = NULL_MOVE;

    search_depth = 5;

    search(b, search_depth, -200000, 200000);

    return best_move;
}

static void init() {
    // if (ms.at == NULL) da_init(&ms, 64);
    ttable_init(&tt, 2<<19);
}

const Player player_v5 = {
    .name = "v5",
    .init = init,
    .eval = eval,
    .select = select_move,
};