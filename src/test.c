#include "chess.h"
#include "term.h"

void print_bitboard(Board* b, u64 bitboard) {
    u8 highlights[64] = {};
    
    for_range(i, 0, 64) {
        u64 bit = bitboard & 1;
        highlights[i] = (u8)bit;
        bitboard = bitboard >> 1;
    }

    print_board(b, highlights);
}

void compile_piece_bitboards(Board* b) {
    u64 black = 0;
    u64 white = 0;

    for_range (i, 0, 64) {
        u64 bit = 1ull << i;
        u8 piece = b->board[i];
        if (piece == EMPTY) continue;
        b->bitboards[piece] |= bit;
    }

    for_range_incl (type, PAWN, KING) {
        white |= b->bitboards[WHITE | type];
        black |= b->bitboards[BLACK | type];
    }

    b->bitboards[WHITE] = white;
    b->bitboards[BLACK] = black;
}

u64 rook_move_board(Board* b, u8 index, bool black_to_move) {
    u8 index_col = index % 8;
    u8 index_row = index / 8;

    u64 rook = 1ll << index;

    u64 col = 0x0101010101010101ull;
    u64 row = 0x00000000000000FFull;
    
    TODO("figure out captures with magic bitboards");

    return ~rook & ((col << index_col) | (row << (index_row*8)));
}

u64 bishop_move_board(Board* b, u8 index, bool black_to_move) {
    u8 index_col = index % 8;
    u8 index_row = index / 8;

    u64 bishop = 1ll << index;

    u64 left  = 0x8040201008040201ull; // left diagonal bitboard
    u64 right = 0x0102040810204080ull; // right diagonal bitboard

    int left_distance = (int)index_col - (int)index_row;

    if (left_distance > 0) left = left >> (left_distance * 8);
    else                   left = left << (-left_distance * 8);

    int right_distance = 7 - (int)index_col - (int)index_row;

    if (right_distance > 0) right = right >> (right_distance * 8);
    else                    right = right << -(right_distance * 8);
    
    TODO("figure out captures with magic bitboards");

    return ~bishop & (right | left);
}

u64 pawn_move_board(Board* b, u8 index, bool black_to_move) {
    
    u8 index_col = index % 8;
    u8 index_row = index / 8;

    u64 pawn = 1ll << index;


    u64 moves = 0;
    u64 ally     = b->bitboards[black_to_move ? BLACK : WHITE];
    u64 opponent = b->bitboards[black_to_move ? WHITE : BLACK];
    u64 occupied = ally | opponent;

    if (black_to_move) {
        moves = pawn << 8 & ~occupied;

        // if is on starting square AND it can make a forward move
        if (index_row == 1 && moves) {
            moves |= pawn << 16 & ~occupied;
        }

        if (index_col != 7) {
            moves |= pawn << 9 & opponent;
        }

        if (index_col != 0) {
            moves |= pawn << 7 & opponent;
        }
    } else {
        moves = pawn >> 8 & ~occupied;

        // if is on starting square AND it can make a forward move
        if (index_row == 6 && moves) {
            moves |= pawn >> 16 & ~occupied;
        }

        if (index_col != 7) {
            moves |= pawn >> 7 & opponent;
        }

        if (index_col != 0) {
            moves |= pawn >> 9 & opponent;
        }
    }

    return moves;
}

u64 knight_move_board(Board* b, u8 index, bool black_to_move) {
    u8 index_col = index % 8;

    u64 knight = 1ll << index;

    u64 moves = 0;
    u64 ally = b->bitboards[black_to_move ? BLACK : WHITE];

    if (index_col != 0) {
        moves |= knight >> 17 & ~ally;
        moves |= knight << 15 & ~ally;
    }
    if (index_col != 7) {
        moves |= knight >> 15 & ~ally;
        moves |= knight << 17 & ~ally;
    }
    if (index_col <= 5) {
        moves |= knight >> 6 & ~ally;
        moves |= knight << 10 & ~ally;
    }
    if (index_col >= 2) {
        moves |= knight << 6 & ~ally;
        moves |= knight >> 10 & ~ally;
    }

    return moves;
}

int main() {
    Board b;
    init_board(&b);
    compile_piece_bitboards(&b);

    MoveSet ms = {};
    da_init(&ms, 10);

    print_board(&b, NULL);
    printf("\n");

    generate_moves(&b, &ms);
}