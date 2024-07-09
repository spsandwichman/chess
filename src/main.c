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

int main() {
    const Player* white = &player_v6;
    const Player* black = &player_v7;

    init_zobrist();

    white->init();
    black->init();

    Board b = {};
    init_board(&b);

    printf("white '%s' \n", white->name);
    printf("black '%s' \n", black->name);
    printf("begin\n");
    print_board(&b, NULL);
    
    int i = 1;
    
    int white_eval = 0;
    int black_eval = 0;


    while (true) {
        // sleep(5);
        Player* player_to_move = b.color_to_move ? black : white;
        Player* opponent = b.color_to_move ? white : black;

        Move move = player_to_move->select(&b, b.color_to_move ? &black_eval : &white_eval);
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

        printf("%d %s :: %s -> %s\n\n",
            i,
            b.color_to_move ? "black" : "white", 
            square_names[move.start], 
            square_names[move.target]);

        make_move(&b, move, true);
        if (history_contains(&b, b.zobrist)) {
            printf("stalemate, position repeated\n");
            break;
        }

        print_board(&b, NULL);

        printf("white eval %+d\n", white_eval);
        printf("black eval %+d\n", black_eval);

        printf("\n");

        if (!b.color_to_move) i++;
    }
    printf("end\n");
}