#include "chess.h"

forceinline static bool is_occupied(Board* b, int i) {
    return piece_type(b->board[i]) != EMPTY;
}

forceinline static bool is_opponent_piece(Board* b, int i) {
    return piece_type(b->board[i]) != EMPTY && b->color_to_move != piece_color(b->board[i]);
}

forceinline static bool can_go_here(Board* b, int i) {
    if (is_occupied(b, i)) return is_opponent_piece(b, i);
    return true;
}

forceinline static void add_move(MoveSet* mv, int from, int to, int* num_moves) {
    (*num_moves)++;
    if (mv == NULL) return;
    Move move = {
        .start = from,
        .target = to,
    };
    da_append(mv, move);
}

forceinline static void add_move_special(MoveSet* mv, int from, int to, int* num_moves, u8 special) {
    (*num_moves)++;
    if (mv == NULL) return;
    Move move = {
        .start = from,
        .target = to,
        .special = special,
    };
    da_append(mv, move);
}

int legal_moves(Board* b, MoveSet* mv) {
    pseudo_legal_moves(b, mv, false);
    return filter_illegal_moves(b, mv);
}

int legal_captures(Board* b, MoveSet* mv) {
    pseudo_legal_moves(b, mv, true);
    return filter_illegal_moves(b, mv);
}

int filter_illegal_moves(Board* b, MoveSet* mv) {
    static MoveSet opponent_responses = {};
    if (opponent_responses.at == NULL) {
        da_init(&opponent_responses, 32);
    }
    da_clear(&opponent_responses);

    swap_color_to_move(*b);
    pseudo_legal_moves(b, &opponent_responses, false);
    swap_color_to_move(*b);

    bool host_in_check = false;
    foreach (Move opp, opponent_responses) {
        if (piece_type(b->board[opp.target]) == KING) {
            host_in_check = true;
            break;
        }
    }

    int legal_moves_count = 0;


    for_range(i, 0, mv->len) {
        Move m = mv->at[i];

        bool skip_move = false;

        // cant castle out of check
        if ((m.special == SPECIAL_KINGSIDE_CASTLE || m.special == SPECIAL_QUEENSIDE_CASTLE) && host_in_check) {
            continue;
        }

        u8 col = b->color_to_move;
        make_move(b, m);
        swap_color_to_move(*b);
        da_clear(&opponent_responses);
        pseudo_legal_moves(b, &opponent_responses, false);

        foreach (Move opp, opponent_responses) {

            // cant castle through check
            if (m.special == SPECIAL_KINGSIDE_CASTLE) {
                if (col == WHITE) {
                    if (opp.target == 7*8 + 5) {
                        skip_move = true;
                        goto skip;
                    }
                } else {
                    if (opp.target == 5) {
                        skip_move = true;
                        goto skip;
                    }
                }
            } else
            if (m.special == SPECIAL_QUEENSIDE_CASTLE) {
                if (col == WHITE) {
                    if (opp.target == 7*8 + 3) {
                        skip_move = true;
                        goto skip;
                    }
                } else {
                    if (opp.target == 3) {
                        skip_move = true;
                        goto skip;
                    }
                }
            }

            // cant move into check
            if (piece_type(b->board[opp.target]) == KING) {
                skip_move = true;
                goto skip;
            }
        }
        skip:
        undo_move(b);
        swap_color_to_move(*b);
        if (!skip_move) {
            mv->at[legal_moves_count] = m;
            legal_moves_count++;
        }
    }

    mv->len = legal_moves_count;
    return legal_moves_count;
}

#define if_capture(target) if (!only_captures || b->board[target] != EMPTY)

int pseudo_legal_moves(Board* b, MoveSet* mv, bool only_captures) {
    int num_moves = 0;
    for_range(i, 0, 64) {
        // skip looking at pieces that arent of the moving color
        if (piece_color(b->board[i]) != b->color_to_move) continue;
        if (piece_type(b->board[i]) == EMPTY) continue;

        int squares_on_left = i % 8;
        int squares_on_right = 7 - squares_on_left;
        int squares_on_top = i / 8;
        int squares_on_bottom = 7 - squares_on_top;

        switch (piece_type(b->board[i])) {
        case PAWN: {
            if (b->color_to_move == WHITE) {
                // diagonal captures
                if (squares_on_left != 0 && squares_on_top != 0 && is_opponent_piece(b, i - 1 - 8)) {
                    if (squares_on_top == 1) {
                        // capture AND promote!!
                        add_move_special(mv, i, i - 1 - 8, &num_moves, SPECIAL_PROMOTE_QUEEN);
                        add_move_special(mv, i, i - 1 - 8, &num_moves, SPECIAL_PROMOTE_ROOK);
                        add_move_special(mv, i, i - 1 - 8, &num_moves, SPECIAL_PROMOTE_BISHOP);
                        add_move_special(mv, i, i - 1 - 8, &num_moves, SPECIAL_PROMOTE_KNIGHT);
                    } else {
                        add_move(mv, i, i - 1 - 8, &num_moves);
                    }
                }
                if (squares_on_right != 0 && squares_on_top != 0 && is_opponent_piece(b, i + 1 - 8)) {
                    if (squares_on_top == 1) {
                        // capture AND promote!!
                        add_move_special(mv, i, i + 1 - 8, &num_moves, SPECIAL_PROMOTE_QUEEN);
                        add_move_special(mv, i, i + 1 - 8, &num_moves, SPECIAL_PROMOTE_ROOK);
                        add_move_special(mv, i, i + 1 - 8, &num_moves, SPECIAL_PROMOTE_BISHOP);
                        add_move_special(mv, i, i + 1 - 8, &num_moves, SPECIAL_PROMOTE_KNIGHT);
                    } else {
                        add_move(mv, i, i + 1 - 8, &num_moves);
                    }
                }
                // en passant
                if (squares_on_left != 0 && ((b->board[i - 1] & 0b1111) == (BLACK | PAWN))) {
                    GameTick last_move = b->move_stack.at[b->move_stack.len - 1];
                    if (last_move.move.special == SPECIAL_PAWN_DOUBLE && last_move.move.target == i - 1) {
                        add_move_special(mv, i, i - 1 - 8, &num_moves, SPECIAL_EN_PASSANT);
                    }
                }
                if (squares_on_right != 0 && ((b->board[i + 1] & 0b1111) == (BLACK | PAWN))) {
                    GameTick last_move = b->move_stack.at[b->move_stack.len - 1];
                    if (last_move.move.special == SPECIAL_PAWN_DOUBLE && last_move.move.target == i + 1) {
                        add_move_special(mv, i, i + 1 - 8, &num_moves, SPECIAL_EN_PASSANT);
                    }
                }

                if (is_occupied(b, i - 8)) {
                    break;
                }
                if (!only_captures && squares_on_top == 1) {
                    add_move_special(mv, i, i - 8, &num_moves, SPECIAL_PROMOTE_QUEEN);
                    add_move_special(mv, i, i - 8, &num_moves, SPECIAL_PROMOTE_ROOK);
                    add_move_special(mv, i, i - 8, &num_moves, SPECIAL_PROMOTE_BISHOP);
                    add_move_special(mv, i, i - 8, &num_moves, SPECIAL_PROMOTE_KNIGHT);
                    break;
                }
                if (!only_captures) add_move(mv, i, i - 8, &num_moves);
                if (!only_captures && i > 47 && !is_occupied(b, i - 16)) { // pawn is on starting square
                    add_move_special(mv, i, i - 16, &num_moves, SPECIAL_PAWN_DOUBLE);
                }
            } else {
                // diagonal captures
                if (squares_on_left != 0 && squares_on_bottom != 0 && is_opponent_piece(b, i - 1 + 8)) {
                    if (squares_on_bottom == 1) {
                        // capture AND promote!!
                        add_move_special(mv, i, i - 1 + 8, &num_moves, SPECIAL_PROMOTE_QUEEN);
                        add_move_special(mv, i, i - 1 + 8, &num_moves, SPECIAL_PROMOTE_ROOK);
                        add_move_special(mv, i, i - 1 + 8, &num_moves, SPECIAL_PROMOTE_BISHOP);
                        add_move_special(mv, i, i - 1 + 8, &num_moves, SPECIAL_PROMOTE_KNIGHT);
                    } else {
                        add_move(mv, i, i - 1 + 8, &num_moves);
                    }
                }
                if (squares_on_right != 0 && squares_on_bottom != 0 && is_opponent_piece(b, i + 1 + 8)) {
                    if (squares_on_bottom == 1) {
                        // capture AND promote!!
                        add_move_special(mv, i, i + 1 + 8, &num_moves, SPECIAL_PROMOTE_QUEEN);
                        add_move_special(mv, i, i + 1 + 8, &num_moves, SPECIAL_PROMOTE_ROOK);
                        add_move_special(mv, i, i + 1 + 8, &num_moves, SPECIAL_PROMOTE_BISHOP);
                        add_move_special(mv, i, i + 1 + 8, &num_moves, SPECIAL_PROMOTE_KNIGHT);
                    } else {
                        add_move(mv, i, i + 1 + 8, &num_moves);
                    }
                }
                // en passant
                if (squares_on_left != 0 && ((b->board[i - 1] & 0b1111) == (WHITE | PAWN))) {
                    GameTick last_move = b->move_stack.at[b->move_stack.len - 1];
                    if (last_move.move.special == SPECIAL_PAWN_DOUBLE && last_move.move.target == i - 1) {
                        add_move_special(mv, i, i - 1 + 8, &num_moves, SPECIAL_EN_PASSANT);
                    }
                }
                if (squares_on_right != 0 && ((b->board[i + 1] & 0b1111) == (WHITE | PAWN))) {
                    GameTick last_move = b->move_stack.at[b->move_stack.len - 1];
                    if (last_move.move.special == SPECIAL_PAWN_DOUBLE && last_move.move.target == i + 1) {
                        add_move_special(mv, i, i + 1 + 8, &num_moves, SPECIAL_EN_PASSANT);
                    }
                }
                if (is_occupied(b, i + 8)) {
                    break;
                }
                if (!only_captures && squares_on_bottom == 1) {
                    add_move_special(mv, i, i + 8, &num_moves, SPECIAL_PROMOTE_QUEEN);
                    add_move_special(mv, i, i + 8, &num_moves, SPECIAL_PROMOTE_ROOK);
                    add_move_special(mv, i, i + 8, &num_moves, SPECIAL_PROMOTE_BISHOP);
                    add_move_special(mv, i, i + 8, &num_moves, SPECIAL_PROMOTE_KNIGHT);
                    break;
                }
                if (!only_captures) add_move(mv, i, i + 8, &num_moves);
                if (!only_captures && i < 16 && !is_occupied(b, i + 16)) { // pawn is on starting square
                    add_move_special(mv, i, i + 16, &num_moves, SPECIAL_PAWN_DOUBLE);
                }
            }
        } break;
        case QUEEN:
        case ROOK: {
            for_range_incl (t, 1, squares_on_left) {
                int target = i - t;
                if (is_occupied(b, target)) {
                    if (is_opponent_piece(b, target)) add_move(mv, i, target, &num_moves);
                    break;
                }
                if (!only_captures) add_move(mv, i, target, &num_moves);
            }
            for_range_incl (t, 1, squares_on_right) {
                int target = i + t;
                if (is_occupied(b, target)) {
                    if (is_opponent_piece(b, target)) add_move(mv, i, target, &num_moves);
                    break;
                }
                if (!only_captures) add_move(mv, i, target, &num_moves);
            }
            for_range_incl (t, 1, squares_on_top) {
                int target = i - t * 8;
                if (is_occupied(b, target)) {
                    if (is_opponent_piece(b, target)) add_move(mv, i, target, &num_moves);
                    break;
                }
                if (!only_captures) add_move(mv, i, target, &num_moves);
            }
            for_range_incl (t, 1, squares_on_bottom) {
                int target = i + t * 8;
                if (is_occupied(b, target)) {
                    if (is_opponent_piece(b, target)) add_move(mv, i, target, &num_moves);
                    break;
                }
                if (!only_captures) add_move(mv, i, target, &num_moves);
            }
            if (piece_type(b->board[i]) != QUEEN) break;
        } 
        case BISHOP: {
            int squares_top_right = min(squares_on_top, squares_on_right);
            int squares_bot_right = min(squares_on_bottom, squares_on_right);
            int squares_top_left  = min(squares_on_top, squares_on_left);
            int squares_bot_left  = min(squares_on_bottom, squares_on_left);

            for_range_incl (t, 1, squares_top_right) {
                int target = i + t - t * 8;
                if (is_occupied(b, target)) {
                    if (is_opponent_piece(b, target)) add_move(mv, i, target, &num_moves);
                    break;
                }
                if (!only_captures) add_move(mv, i, target, &num_moves);
            }
            for_range_incl (t, 1, squares_top_left) {
                int target = i - t - t * 8;
                if (is_occupied(b, target)) {
                    if (is_opponent_piece(b, target)) add_move(mv, i, target, &num_moves);
                    break;
                }
                if (!only_captures) add_move(mv, i, target, &num_moves);
            }
            for_range_incl (t, 1, squares_bot_right) {
                int target = i + t + t * 8;
                if (is_occupied(b, target)) {
                    if (is_opponent_piece(b, target)) add_move(mv, i, target, &num_moves);
                    break;
                }
                if (!only_captures) add_move(mv, i, target, &num_moves);
            }
            for_range_incl (t, 1, squares_bot_left) {
                int target = i - t + t * 8;
                if (is_occupied(b, target)) {
                    if (is_opponent_piece(b, target)) add_move(mv, i, target, &num_moves);
                    break;
                }
                if (!only_captures) add_move(mv, i, target, &num_moves);
            }
        } break;
        case KNIGHT: {
            // quite possibly stupid
            if (squares_on_top    >= 2 && squares_on_left  >= 1 && can_go_here(b, i - 1 - 8 * 2)) if_capture(i - 1 - 8 * 2) add_move(mv, i, i - 1 - 8 * 2, &num_moves);
            if (squares_on_top    >= 2 && squares_on_right >= 1 && can_go_here(b, i + 1 - 8 * 2)) if_capture(i + 1 - 8 * 2) add_move(mv, i, i + 1 - 8 * 2, &num_moves);
            if (squares_on_bottom >= 2 && squares_on_left  >= 1 && can_go_here(b, i - 1 + 8 * 2)) if_capture(i - 1 + 8 * 2) add_move(mv, i, i - 1 + 8 * 2, &num_moves);
            if (squares_on_bottom >= 2 && squares_on_right >= 1 && can_go_here(b, i + 1 + 8 * 2)) if_capture(i + 1 + 8 * 2) add_move(mv, i, i + 1 + 8 * 2, &num_moves);
            if (squares_on_top    >= 1 && squares_on_left  >= 2 && can_go_here(b, i - 2 - 8 * 1)) if_capture(i - 2 - 8 * 1) add_move(mv, i, i - 2 - 8 * 1, &num_moves);
            if (squares_on_top    >= 1 && squares_on_right >= 2 && can_go_here(b, i + 2 - 8 * 1)) if_capture(i + 2 - 8 * 1) add_move(mv, i, i + 2 - 8 * 1, &num_moves);
            if (squares_on_bottom >= 1 && squares_on_left  >= 2 && can_go_here(b, i - 2 + 8 * 1)) if_capture(i - 2 + 8 * 1) add_move(mv, i, i - 2 + 8 * 1, &num_moves);
            if (squares_on_bottom >= 1 && squares_on_right >= 2 && can_go_here(b, i + 2 + 8 * 1)) if_capture(i + 2 + 8 * 1) add_move(mv, i, i + 2 + 8 * 1, &num_moves);
        } break;
        case KING: {
            // printf("\n\n%d %d %d %d\n\n", squares_on_top, squares_on_bottom, squares_on_left, squares_on_right);
            if (squares_on_top    >= 1 && can_go_here(b, i - 8)) if_capture(i - 8) add_move(mv, i, i - 8, &num_moves);
            if (squares_on_bottom >= 1 && can_go_here(b, i + 8)) if_capture(i + 8) add_move(mv, i, i + 8, &num_moves);
            if (squares_on_left   >= 1 && can_go_here(b, i - 1)) if_capture(i - 1) add_move(mv, i, i - 1, &num_moves);
            if (squares_on_right  >= 1 && can_go_here(b, i + 1)) if_capture(i + 1) add_move(mv, i, i + 1, &num_moves);
        
            if (squares_on_top    >= 1 && squares_on_right >= 1 && can_go_here(b, i - 8 + 1)) if_capture(i - 8 + 1) add_move(mv, i, i - 8 + 1, &num_moves);
            if (squares_on_top    >= 1 && squares_on_left  >= 1 && can_go_here(b, i - 8 - 1)) if_capture(i - 8 - 1) add_move(mv, i, i - 8 - 1, &num_moves);
            if (squares_on_bottom >= 1 && squares_on_right >= 1 && can_go_here(b, i + 8 + 1)) if_capture(i + 8 + 1) add_move(mv, i, i + 8 + 1, &num_moves);
            if (squares_on_bottom >= 1 && squares_on_left  >= 1 && can_go_here(b, i + 8 - 1)) if_capture(i + 8 - 1) add_move(mv, i, i + 8 - 1, &num_moves);
        } break;
        default:
            break;
        }
    }
    // report castle
    if (!only_captures) {
        if (b->color_to_move == WHITE) {
            if (b->board[7*8 + 0] == (WHITE | ROOK) &&
                b->board[7*8 + 1] == (EMPTY) &&
                b->board[7*8 + 2] == (EMPTY) &&
                b->board[7*8 + 3] == (EMPTY) &&
                b->board[7*8 + 4] == (WHITE | KING)) {
                    add_move_special(mv, 7*8 + 4, 7*8 + 2, &num_moves, SPECIAL_QUEENSIDE_CASTLE);
            }
            if (b->board[7*8 + 7] == (WHITE | ROOK) &&
                b->board[7*8 + 6] == (EMPTY) &&
                b->board[7*8 + 5] == (EMPTY) &&
                b->board[7*8 + 4] == (WHITE | KING)) {
                    add_move_special(mv, 7*8 + 4, 7*8 + 6, &num_moves, SPECIAL_KINGSIDE_CASTLE);
            }
        } else {
            if (b->board[0] == (BLACK | ROOK) &&
                b->board[1] == (EMPTY) &&
                b->board[2] == (EMPTY) &&
                b->board[3] == (EMPTY) &&
                b->board[4] == (BLACK | KING)) {
                    add_move_special(mv, 4, 2, &num_moves, SPECIAL_QUEENSIDE_CASTLE);
            }
            if (b->board[7] == (BLACK | ROOK) &&
                b->board[6] == (EMPTY) &&
                b->board[5] == (EMPTY) &&
                b->board[4] == (BLACK | KING)) {
                    add_move_special(mv, 4, 6, &num_moves, SPECIAL_KINGSIDE_CASTLE);
            }
        }
    }

    return num_moves;
}

void make_move(Board* b, Move mv) {
    // printf("MOVE start %s target %s special %d\n", square_names[mv.start], square_names[mv.target], mv.special);

    GameTick gt = {};
    gt.move = mv;
    gt.white_move = b->color_to_move == WHITE;

    if (b->board[mv.target] != EMPTY) {
        gt.captured = b->board[mv.target];
        gt.capture_location = mv.target;
    }

    if (!(b->board[mv.start] & HAS_MOVED)) {
        gt.piece_first_move = true;
    }

    b->board[mv.target] = b->board[mv.start] | HAS_MOVED;
    b->board[mv.start] = EMPTY;


    switch (mv.special) {
    case SPECIAL_KINGSIDE_CASTLE:
        if (piece_color(b->board[mv.target]) == WHITE) {
            b->board[7*8 + 7] = EMPTY;
            b->board[7*8 + 5] = (WHITE | ROOK | HAS_MOVED);
        } else {
            b->board[7] = EMPTY;
            b->board[5] = (BLACK | ROOK | HAS_MOVED);
        }
        break;
    case SPECIAL_QUEENSIDE_CASTLE:
        if (piece_color(b->board[mv.target]) == WHITE) {
            b->board[7*8 + 0] = EMPTY;
            b->board[7*8 + 3] = (WHITE | ROOK | HAS_MOVED);
        } else {
            b->board[0] = EMPTY;
            b->board[3] = (BLACK | ROOK | HAS_MOVED);
        }
        break;
    case SPECIAL_EN_PASSANT:
        if (piece_color(b->board[mv.target]) == WHITE) {
            gt.captured = b->board[mv.target + 8];
            gt.capture_location = mv.target + 8;
            b->board[mv.target + 8] = EMPTY;
        } else {
            gt.captured = b->board[mv.target - 8];
            gt.capture_location = mv.target - 8;
            b->board[mv.target - 8] = EMPTY;
        }
        break;
    case SPECIAL_PROMOTE_QUEEN:  
        b->board[mv.target] = (b->board[mv.target] & 0b11111000) | QUEEN; 
        break;
    case SPECIAL_PROMOTE_ROOK:   
        b->board[mv.target] = (b->board[mv.target] & 0b11111000) | ROOK; 
        break;
    case SPECIAL_PROMOTE_BISHOP: b->board[mv.target] = (b->board[mv.target] & 0b11111000) | BISHOP; break;
    case SPECIAL_PROMOTE_KNIGHT: b->board[mv.target] = (b->board[mv.target] & 0b11111000) | KNIGHT; break;
    
    default:
        break;
    }

    if (b->move_stack.at == NULL) {
        da_init(&b->move_stack, 64);
        da_clear(&b->move_stack);
    }
    // printf("%p %d %d\n", b->move_stack.at, b->move_stack.len, b->move_stack.cap);
    da_append(&b->move_stack, gt);
    // swap_color_to_move(*b);
}

void undo_move(Board* b) {
    if (b->move_stack.len == 0) return;

    // swap_color_to_move(*b);
    GameTick gt = b->move_stack.at[b->move_stack.len - 1];
    da_pop(&b->move_stack);

    // if (print) printf("UNDO start %s target %s special %d captured %b captured_loc %s\n",
    //     square_names[gt.move.start],
    //     square_names[gt.move.target],
    //     gt.move.special,
    //     gt.captured,
    //     square_names[gt.capture_location]);

    b->board[gt.move.start] = b->board[gt.move.target];
    b->board[gt.move.target] = EMPTY;

    if (gt.captured) {
        b->board[gt.capture_location] = gt.captured;
    }

    switch (gt.move.special) {
    case SPECIAL_PROMOTE_BISHOP:
    case SPECIAL_PROMOTE_QUEEN:
    case SPECIAL_PROMOTE_KNIGHT:
    case SPECIAL_PROMOTE_ROOK:
        b->board[gt.move.start] = (b->board[gt.move.start] & 0b11111000) | PAWN;
        break;
    case SPECIAL_KINGSIDE_CASTLE:
        if (gt.white_move) {
            b->board[7*8 + 5] = EMPTY;
            b->board[7*8 + 7] = WHITE | ROOK;
        } else {
            b->board[5] = EMPTY;
            b->board[7] = BLACK | ROOK;
        }
        break;
    case SPECIAL_QUEENSIDE_CASTLE:
        if (gt.white_move) {
            b->board[7*8 + 3] = EMPTY;
            b->board[7*8 + 0] = WHITE | ROOK;
        } else {
            b->board[3] = EMPTY;
            b->board[0] = BLACK | ROOK;
        }
        break;
    
    default:
        break;
    }

    if (gt.piece_first_move) {
        b->board[gt.move.start] = b->board[gt.move.start] & 0b11101111;
    }

    // b->color_to_move = gt.white_move ? BLACK : WHITE;
}

int indexof(char* s) {
    int m = 0;
    switch (s[0]) {
    case 'a': m = 0; break;
    case 'b': m = 1; break;
    case 'c': m = 2; break;
    case 'd': m = 3; break;
    case 'e': m = 4; break;
    case 'f': m = 5; break;
    case 'g': m = 6; break;
    case 'h': m = 7; break;
    default: return -1;
    }
    switch (s[1]) {
    case '8': m += 0; break;
    case '7': m += 1 * 8; break;
    case '6': m += 2 * 8; break;
    case '5': m += 3 * 8; break;
    case '4': m += 4 * 8; break;
    case '3': m += 5 * 8; break;
    case '2': m += 6 * 8; break;
    case '1': m += 7 * 8; break;
    default: return -1;
    }
    return m;
}

char* square_names[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};