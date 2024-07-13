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

    u64 rook = 1ull << index;

    u64 col = 0x0101010101010101ull;
    u64 row = 0x00000000000000FFull;
    
    TODO("figure out captures with magic bitboards");

    return ~rook & ((col << index_col) | (row << (index_row*8)));
}

u64 bishop_move_board(Board* b, u8 index, bool black_to_move) {
    u8 index_col = index % 8;
    u8 index_row = index / 8;

    u64 bishop = 1ull << index;

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

    u64 pawn = 1ull << index;


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

    u64 knight = 1ull << index;

    u64 moves = 0;
    u64 ally = b->bitboards[black_to_move ? BLACK : WHITE];

    if (index_col != 0) {
        moves |= knight >> 17;
        moves |= knight << 15;
    }
    if (index_col != 7) {
        moves |= knight >> 15;
        moves |= knight << 17;
    }
    if (index_col <= 5) {
        moves |= knight >> 6;
        moves |= knight << 10;
    }
    if (index_col >= 2) {
        moves |= knight << 6;
        moves |= knight >> 10;
    }

    return moves & ~ally;
}

u64 king_move_board(Board* b, u8 index, bool black_to_move) {
    u8 index_col = index % 8;

    u64 ally = b->bitboards[black_to_move ? BLACK : WHITE];
    u64 king = 1ull << index;
    u64 moves = 0;

    moves |= king << 8;
    moves |= king >> 8;

    if (index_col != 7) {
        moves |= king << 1;
        moves |= king << 9;
        moves |= king >> 7;
    }
    if (index_col != 0) {
        moves |= king >> 1;
        moves |= king >> 9;
        moves |= king << 7;
    }

    if (black_to_move) {
        if (b->black_can_kingside_castle)  moves |= 1ull << (6);
        if (b->black_can_queenside_castle) moves |= 1ull << (2);
    } else {
        if (b->white_can_kingside_castle)  moves |= 1ull << (7*8 + 6);
        if (b->white_can_queenside_castle) moves |= 1ull << (7*8 + 2);
    }

    return moves & ~ally;
}

u64 pseudo_attack_board(Board* b, bool black_is_attacking) {
    u64 board = 0;
    for_range(i, 0, 64) {
        if (piece_color(b->board[i]) != (black_is_attacking ? BLACK : WHITE)) {
            continue;
        }
        switch (piece_type(b->board[i])) {
        case EMPTY: 
            continue;
        case PAWN:
            board |= pawn_move_board(b, i, black_is_attacking);
            break;
        case ROOK:
            board |= rook_move_board(b, i, black_is_attacking);
            break;
        case BISHOP:
            board |= bishop_move_board(b, i, black_is_attacking);
            break;
        case QUEEN:
            board |= bishop_move_board(b, i, black_is_attacking);
            board |= rook_move_board(b, i, black_is_attacking);
            break;
        case KING:
            board |= king_move_board(b, i, black_is_attacking);
            break;

        default:
            break;
        }
    }
    return board;
}

u64 possible_moves_bitboard(Board* b, bool black_to_move) {
    return 0;
}

int main() {
    Board b;
    init_board(&b);
    compile_piece_bitboards(&b);

    MoveSet ms = {};
    da_init(&ms, 10);

    print_board(&b, NULL);
    printf("\n");

    int index = indexof("e1");

    print_bitboard(&b, king_move_board(&b, index, false));

    // generate_moves(&b, &ms);
}