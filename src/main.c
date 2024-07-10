#include "chess.h"
#include <time.h>
#include <unistd.h>

void clear_screen() {
    // const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
    // write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
    printf("\e[1;1H\e[2J");
}

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

void print_board_with_move(Board* b, Move move) {
    u8 highlights[64] = {};
    if (!is_move_null(move)) {
        highlights[move.start] = 2;
        highlights[move.target] = 3;
    }
    print_board(b, highlights);
}

void print_move(Board* b, int i, Move move) {
    printf("%d %s :: %s -> %s\n",
        i,
        b->color_to_move ? "black" : "white", 
        square_names[move.start], 
        square_names[move.target]);
}

int main() {
    clear_screen();
    
    const Player* white = &player_first;
    const Player* black = &player_v7;

    init_zobrist();

    white->init();
    black->init();

    Board b = {};
    init_board(&b);

    printf("white '%s' vs black '%s'\n", white->name, black->name);

    printf("begin\n\n");
    print_board(&b, NULL);
    
    int i = 1;
    
    int white_eval = 0;
    int black_eval = 0;
    int last_white_eval = 0;
    int last_black_eval = 0;

    MoveSet possible_moves = {};
    da_init(&possible_moves, 64);

    Move last_move = NULL_MOVE;
    int last_i = 1;

    while (true) {
        da_clear(&possible_moves);
        // sleep(5);
        Player* player_to_move = b.color_to_move ? black : white;
        Player* opponent = b.color_to_move ? white : black;


        Move move = player_to_move->select(&b, b.color_to_move ? &black_eval : &white_eval);
        clear_screen();
        printf("white '%s' vs black '%s'\n", white->name, black->name);


        if (is_move_null(last_move)) {
            printf("\n\n");
        } else {
            swap_color_to_move(b);
            print_move(&b, last_i, last_move);
            swap_color_to_move(b);
            printf("\n");
        }

        print_board_with_move(&b, last_move);
        printf("white eval %+d\n", last_white_eval);
        printf("black eval %+d\n", last_black_eval);
        printf("\n");

        legal_moves(&b, &possible_moves);

        // engine returned null move
        if (is_move_null(move)) {
            if (possible_moves.len != 0) {
                printf("error, %s '%s' returned null move", b.color_to_move ? "black" : "white", opponent->name);
            } else if (is_king_in_check(&b, KING, b.color_to_move)) {
                // checkmate
                printf("checkmate, %s '%s' wins\n", b.color_to_move ? "black" : "white", opponent->name);
            } else {
                // stalemate
                printf("stalemate, no available moves for %s\n", b.color_to_move ? "white" : "black");
            }
            break;
        }
        if (is_two_king_draw(&b)) {
            printf("stalemate, two-king draw\n");
            break;
        }

        print_move(&b, i, move);
        printf("\n");

        make_move(&b, move, true);

        print_board_with_move(&b, move);

        printf("white eval %+d\n", white_eval);
        printf("black eval %+d\n", black_eval);
        printf("\n");

        if (history_contains(&b, b.zobrist)) {
            printf("stalemate, position repeated\n");
            break;
        }

        last_white_eval = white_eval;
        last_black_eval = black_eval;
        last_move = move;
        last_i = i;
        if (!b.color_to_move) i++;
    }
    printf("end\n");
}