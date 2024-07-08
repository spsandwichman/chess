#include "chess.h"
#include <time.h>
#include <unistd.h>

bool is_two_king_draw(Board* b) {
    for_range(i, 0, 64) {
        if (piece_type(b->board[i]) != KING && piece_type(b->board[i]) != EMPTY) return false;
    }
    return true;
}

bool is_king_in_check(Board* b, u8 piece, u8 color) {
    piece = piece | color;
    
    MoveSet opp_moves = {};
    da_init(&opp_moves, 32);
    
    b->color_to_move = color == WHITE ? BLACK : WHITE;
    pseudo_legal_moves(b, &opp_moves, true);
    filter_illegal_moves(b, &opp_moves);

    foreach (Move m, opp_moves) {
        if ((b->board[m.target] & 0b01111) == piece) {
            return true;
        }
    }

    da_destroy(&opp_moves);
    return false;
}

const u8 starting_board[64] = {
    BLACK | ROOK, BLACK | KNIGHT, BLACK | BISHOP, BLACK | QUEEN, BLACK | KING, BLACK | BISHOP, BLACK | KNIGHT, BLACK | ROOK,
    BLACK | PAWN, BLACK | PAWN,   BLACK | PAWN,   BLACK | PAWN,  BLACK | PAWN, BLACK | PAWN,   BLACK | PAWN,   BLACK | PAWN,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    WHITE | PAWN, WHITE | PAWN,   WHITE | PAWN,   WHITE | PAWN,  WHITE | PAWN, WHITE | PAWN,   WHITE | PAWN,   WHITE | PAWN,
    WHITE | ROOK, WHITE | KNIGHT, WHITE | BISHOP, WHITE | QUEEN, WHITE | KING, WHITE | BISHOP, WHITE | KNIGHT, WHITE | ROOK,
};

int main() {
    init_zobrist();

    const Player* white = &player_v4;
    const Player* black = &player_v3;

    white->init();
    black->init();

    Board b = {};
    init_board(&b);

    printf("white '%s' \n", white->name);
    printf("black '%s' \n", black->name);
    printf("begin\n");
    print_board(&b, NULL);
    
    int i = 1;
    while (true) {
        Player* player_to_move = b.color_to_move ? black : white;
        Player* opponent = b.color_to_move ? white : black;

        Move move = player_to_move->select(&b);
        if (is_two_king_draw(&b)) {
            printf("stalemate, two-king draw\n");
            break;
        }

        // no moves left
        if (is_move_null(move)) {
            if (is_king_in_check(&b, KING, b.color_to_move)) {
                // checkmate
                printf("checkmate, %s '%s' wins\n", b.color_to_move ? "black" : "white", opponent->name);
            } else {
                // stalemate
                printf("stalemate, no available moves for %s\n", b.color_to_move ? "white" : "black");
            }
            break;
        }

        printf("\n%d %s :: %s -> %s\n\n",
            i,
            b.color_to_move ? "black" : "white", 
            square_names[move.start], 
            square_names[move.target]);
    
        make_move(&b, move);
        swap_color_to_move(b);

        print_board(&b, NULL);
        if (!b.color_to_move) i++;
    }
    printf("end\n");
}


/*

typedef struct PerftInfo {
    u64 sum;
    u64 captures;
    u64 castles;
    u64 en_passants;
} PerftInfo;

void movegen(Board* b, PerftInfo* info, int depth) {

    MoveSet ms = {};
    da_init(&ms, 64);

    legal_moves(b, &ms);

    foreach (Move m, ms) {
        
        if (depth == 1) {
            info->sum++;
            switch (m.special) {
            case SPECIAL_KINGSIDE_CASTLE:
            case SPECIAL_QUEENSIDE_CASTLE:
                info->castles++;
                break;
            case SPECIAL_EN_PASSANT:
                info->en_passants++;
                break;
            default: break;
            }
        }

        make_move(b, m);

        // if (m.special) {
        //     printf("\n\n%s -> %s %d\n", square_names[m.start], square_names[m.target], m.special);
        //     print_board(b, NULL);
        // }

        if (depth == 1 && b->move_stack.at[b->move_stack.len-1].captured) {
            info->captures++;
        }

        if (depth != 1) {
            swap_color_to_move(*b);
            movegen(b, info, depth - 1);
            swap_color_to_move(*b);
        }
        undo_move(b);
    }

    da_destroy(&ms);
    return;
}

void movegen_test(int depth) {
    Board b = {};
    init_board(&b);
    load_board(&b, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");
    print_board(&b, NULL);
    print_board_debug(&b);

    PerftInfo info = {};

    movegen(&b, &info, depth);

    printf("depth %d ", depth);
    printf("moves %zu ", info.sum);
    printf("captures %zu ", info.captures);
    printf("en_passants %zu ", info.en_passants);
    printf("castles %zu\n", info.castles);

}

int main() {
    movegen_test(4);
}
*/