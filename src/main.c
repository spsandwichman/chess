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
    
    MoveSet opp_moves = {0};
    da_init(&opp_moves, 32);
    
    b->color_to_move = color == WHITE ? BLACK : WHITE;
    pseudo_legal_moves(b, &opp_moves);
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
    const Player* white = &player_random;
    const Player* black = &player_v1;

    white->init();
    black->init();

    Board b = {0};
    init_board(&b);

    printf("white '%s' \n", white->name);
    printf("black '%s' \n", black->name);
    printf("begin\n");
    print_board(&b, NULL);
    
    int i = 1;
    while (true) {
        usleep(10000);
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

        printf("%d %s :: %s -> %s\n",
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
int main() {
    struct timeval time = {};
    gettimeofday(&time, NULL);
    srand((int)time.tv_sec);
    
    int game = 1;
    int checkmates = 0;
    while (true) {
    for (;; game++) {
        Board b = {0};
        init_board(&b);

        MoveSet ms = {};
        da_init(&ms, 64);

        print_board_debug(&b);

        print_board(&b, NULL);

        da_clear(&ms);
        pseudo_legal_moves(&b, &ms);
        filter_illegal_moves(&b, &ms);

        print_board_w_moveset(&b, &ms);
        
        undo_move(&b);
        undo_move(&b);
        undo_move(&b);
        undo_move(&b);
        undo_move(&b);
        undo_move(&b);

        print_board_debug(&b);

        return 0; // EARLY RETURN FOR TESTING
    



        int movecount = 1;
        for (;movecount < 2000; b.color_to_move ? (1) : (movecount++)) {
            da_clear(&ms);
            int num_moves = pseudo_legal_moves(&b, &ms);
            num_moves = filter_illegal_moves(&b, &ms);

            printf("\e[1;1H\e[2J");
            // print_board(&b, NULL);
            // printf("\n");
            // print_board_w_moveset(&b, &ms);
            printf("\ngame %d move %d %s to move", game, movecount, b.color_to_move ? "black" : "white");
            if (num_moves == 0) {
                if (is_king_in_check(&b, KING, b.color_to_move)) {
                    printf(" :: checkmate (%s wins)\n", b.color_to_move ? "black" : "white");
                    printf("%d (%f%%) checkmates\n", checkmates, (100.0f*checkmates/game));
                    goto newgame;
                } else {
                    printf(" :: stalemate\n");
                    printf("%d (%f%%) checkmates\n", checkmates, (100.0f*checkmates/game));
                }
                break;
            }
            if (is_two_king_draw(&b)) {
                printf(":: stalemate\n");
                printf("%d (%f%%) checkmates\n", checkmates, (100.0f*checkmates/game));
                break;
            }
            printf("\n");
            printf("%d (%f%%) checkmates\n", checkmates, (100.0f*checkmates/game));


            int move_index = rand() % num_moves;
            Move move = ms.at[move_index];
            make_move(&b, move);
            b.color_to_move = b.color_to_move == WHITE ? BLACK : WHITE;
            // usleep(2000000);
            usleep(10);
        }
        usleep(10);
    }
    newgame:
    checkmates++;
    // usleep(50000);
    }
}*/