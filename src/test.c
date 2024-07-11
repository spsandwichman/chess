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

int main() {
    Board b;
    init_board(&b);

    MoveSet ms = {};
    da_init(&ms, 10);

    generate_moves(&b, &ms);
}