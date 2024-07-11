#include "chess.h"

int generate_moves(Board* b, MoveSet* ms) {
    assert(!b->black_to_move);

    

    return 0;
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

int pseudo_legal_moves(Board* b, MoveSet* mv, bool only_captures)   { TODO(""); }
int filter_illegal_moves(Board* b, MoveSet* mv)                     { TODO(""); }
int legal_moves(Board* b, MoveSet* mv)                              { TODO(""); }
int legal_captures(Board* b, MoveSet* mv)                           { TODO(""); }
void make_move(Board* b, Move mv, bool swap_colors)                 { TODO(""); }
void undo_move(Board* b, bool swap_colors)                          { TODO(""); }