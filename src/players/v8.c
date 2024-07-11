#include "chess.h"

#define SEARCH_BUFFER_LEN 256
#define QSEARCH_BUFFER_LEN 128

// #define LOG(...) printf(__VA_ARGS__)
#define LOG(...)

static const int milliseconds_allotted = 1000;

static const int checkmate_score = -100000;

// #define FUCKING_HATE_STALEMATES
#ifdef FUCKING_HATE_STALEMATES
static const int stalemate_score = -100000;
#else
static const int stalemate_score = 0;
#endif

static TransposTable* tt;
static TransposTable  black_tt;
static TransposTable  white_tt;

static int piece_value(u8 kind) {
    switch (kind) {
    case KING: return 5000;
    case QUEEN: return 90;
    case ROOK: return 50;
    case BISHOP: return 30;
    case KNIGHT: return 30;
    case PAWN: return 10;
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
        int offset = piece_color(p) == b->black_to_move ? 1 : -1;
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
        piece_value(KING)   * diff_kings +
        piece_value(QUEEN)  * diff_queens +
        piece_value(ROOK)   * diff_rooks +
        piece_value(BISHOP) * diff_bishops +
        piece_value(KNIGHT) * diff_knights +
        piece_value(PAWN)   * diff_pawns +
        2                   * diff_mobility;
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
    Move backing_buffer[QSEARCH_BUFFER_LEN];
    ms.at = backing_buffer;
    ms.cap = QSEARCH_BUFFER_LEN;
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
static Move best_move_iter;
static int  best_eval;
static int  best_eval_iter;

static int  ttable_hits = 0;
static int  ttable_misses = 0;

static bool search_cancelled = false;
// static struct timespec ts_start;
static u64 start_milliseconds;

static int search(Board* b, int depth, int search_depth, int alpha, int beta) {

    if (search_cancelled) return 0;

    if (depth >= search_depth) { // bottom
        return q_search(b, alpha, beta);
    }

    if (depth != 0) {
        // avoid stalemate by repetition
        if (history_contains(b, b->zobrist)) {
            return stalemate_score;
        }
    }

    if (depth != 0) {
        TransposEntry* entry = ttable_get(tt, b->zobrist, depth, alpha, beta);
        if (entry == NULL) {
            ttable_misses++;
        }
        if (entry != NULL) {
            ttable_hits++;
            return entry->eval; 
        }
    }
    

    u8 bound = TT_UPPER;
    
    MoveSet moveset = {};
    Move backing_buffer[SEARCH_BUFFER_LEN];

    moveset.cap = SEARCH_BUFFER_LEN;
    // BAD BAD BAD BAD
    moveset.at = backing_buffer;
    moveset.len = 0;

    MoveSet* ms = &moveset;

    legal_moves(b, ms);

    if (ms->len == 0) {
        // test if we're in check
        bool in_check = false;
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
            return checkmate_score + depth; // checkmated
        } else {
            return stalemate_score; // stalemated
        }
    }

    if (depth == 0 && !is_move_null(best_move)) {

        make_move(b, best_move, true);
        int evaluation = -search(b, depth + 1, search_depth, -beta, -alpha);
        undo_move(b, true);

        if (search_cancelled) return 0;

        if (evaluation >= beta) {
            ttable_put(tt, b->zobrist, beta, depth, TT_LOWER);
            return beta;
        }

        // found a new best move!
        if (evaluation > alpha) {
            alpha = evaluation;

            if (depth == 0) {
                best_move_iter = best_move;
                best_eval_iter = evaluation;
            }

            bound = TT_EXACT;
        }
    }

    {
        struct timespec ts_current;
        clock_gettime(CLOCK_MONOTONIC, &ts_current);
        u64 current_milliseconds = (u64)ts_current.tv_sec * 1000;
        current_milliseconds += (u64)ts_current.tv_nsec / 1000000;
        if (current_milliseconds - start_milliseconds >= milliseconds_allotted) {
            search_cancelled = true;
        }
    }

    order_moves(b, ms);

    for_range(i, 0, ms->len) {
        Move m = ms->at[i];

        if (depth == 0 && m.start == best_move.start && m.target == best_move.target) continue;

        int evaluation;
        make_move(b, m, true);

        if (search_depth - depth > 4 && i >= ((2*ms->len)/3)) {
            // search a little smaller
            evaluation = -search(b, depth + 1, search_depth - 1, -beta, -alpha);

            // position might be better than expected, expore this further
            if (evaluation > alpha) goto standard_eval;
        } else {
            standard_eval:
            evaluation = -search(b, depth + 1, search_depth, -beta, -alpha);
        }

        undo_move(b, true);

        if (search_cancelled) return 0;

        if (evaluation >= beta) {
            ttable_put(tt, b->zobrist, beta, depth, TT_LOWER);
            return beta;
        }

        // found a new best move!
        if (evaluation > alpha) {
            alpha = evaluation;

            if (depth == 0) {
                best_move_iter = m;
                best_eval_iter = evaluation;
            }

            bound = TT_EXACT;
        }
    }

    ttable_put(tt, b->zobrist, alpha, depth, bound);
    return alpha;
}



static int iterative_deepening_search(Board* b) {
    search_cancelled = false;
    best_move = best_move_iter = NULL_MOVE;
    best_eval = best_eval_iter = INT_MIN;

    memset(tt->at, 0, tt->len * sizeof(tt->at[0]));

    struct timespec ts_start;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    start_milliseconds = ((u64)ts_start.tv_sec * 1000) + ((u64)ts_start.tv_nsec / 1000000);

    // hard limit at 200 search iterations 
    for_range_incl(d, 1, 200) {

        best_move_iter = NULL_MOVE;
        best_eval_iter = INT_MIN;
        
        search(b, 0, d, -400000, 400000);

        if (best_eval_iter != INT_MIN) LOG("[V8] iter %d candidate %s -> %s with eval %d\n", d, square_names[best_move_iter.start], square_names[best_move_iter.target], best_eval_iter);
        
        if (!is_move_null(best_move_iter)) {

            best_move = best_move_iter;
            best_eval = best_eval_iter;

            // if (best_eval == checkmate_score) {
            //     break; // go for the kill
            // }
        }

        if (search_cancelled) {
            LOG("[V8] search cancelled\n", d);
            break;
        }
    }

    return best_eval;
}

static Move select_move(Board* b, int* eval_out) {

    // choose which transposition table to use.
    // used to use just one table, caused problems with mutliple players evaluating positions incorrectly
    tt = b->black_to_move ? &black_tt : &white_tt;
    if (tt->at == NULL) ttable_init(tt, 1<<20);

    ttable_hits = 0;
    ttable_misses = 0;

    best_move = best_move_iter = NULL_MOVE;
    best_eval = best_eval_iter = INT_MIN;

    // int search_depth = 5;

    iterative_deepening_search(b);
    *eval_out = best_eval;

    LOG("[V8] transposition table hits : %d/%d (%f)\n", ttable_hits, ttable_hits+ttable_misses, (ttable_hits*100.0f/(ttable_hits+ttable_misses)));

    return best_move;
}

static void init() {
    // if (ms.at == NULL) da_init(&ms, 64);
}

const Player player_v8 = {
    .name = "v8",
    .init = init,
    .eval = eval,
    .select = select_move,
};