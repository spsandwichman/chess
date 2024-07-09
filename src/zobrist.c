#include "chess.h"

// zobrist hashing

// 64 positions, 32 possible pieces
u64 zobrist_values[64 * 32] = {};
u64 zobrist_black_to_move = 0;

void init_zobrist() {
    init_genrand64(0xDEADBEEFDEADBEEFull);

    for_range (pos, 0, 64) {
        for_range (piece, 1, 32) {
            zobrist_values[pos * 32 + piece] = genrand64_int64();
        }
    }
    zobrist_black_to_move = genrand64_int64();
}

u64 zobrist_component(u8 piece, u8 position) {
    return zobrist_values[position * 32 + piece];
}

u64 zobrist_full_board(Board* b) {
    u64 hash = b->color_to_move ? zobrist_black_to_move : 0;
    for_urange(i, 0, 64) {
        hash ^= zobrist_component(b->board[i], i);
    }
    return hash;
}