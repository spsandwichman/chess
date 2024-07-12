#include "chess.h"

void print_bitboard(u64 bitboard) {
    for_range(i, 0, 8) {
        for_range(j, 0, 8) {
            u64 bit = bitboard & 1;
            printf("%d ", bit);
            bitboard = bitboard >> 1;
        }
        printf("\n");
    }
}

u64 rook_attack_board(u8 index) {
    u8 index_col = index % 8;
    u8 index_row = index / 8;

    u64 rook = 1ll << index;

    u64 col = 0x0101010101010101ull;
    u64 row = 0x00000000000000FFull;
    
    return ~rook & ((col << index_col) | (row << (index_row*8)));
}

u64 bishop_attack_board(u8 index) {
    u8 index_col = index % 8;
    u8 index_row = index / 8;

    u64 bishop = 1ll << index;

    int left_distance = (int)index_col - (int)index_row;

    u64 left  = 0x8040201008040201ull;
    u64 right = 0x0102040810204080ull;

    if (left_distance > 0) {
        left = left << left_distance;
    } else {
        left = left >> -left_distance;
    }
    return left;
    
    // return ~rook & ((col << index_col) | (row << (index_row*8)));
}

int main() {
    Board b;
    init_board(&b);

    MoveSet ms = {};
    da_init(&ms, 10);

    u8 index = 8;

    print_bitboard(1ll << index); printf("\n");
    print_bitboard(bishop_attack_board(index));

    generate_moves(&b, &ms);
}