#include "chess.h"
#include "term.h"

#define at(row, col) (b->board[(row) * 8 + (col)])

void init_board(Board* b) {
    load_board(b, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w");
    da_init(&b->move_stack, 16);
}

void load_board(Board* b, char* fen) {

    memset(b->board, 0, 64);

    // assuming well-formed fen strings
    char* cursor = fen;
    for_range(row, 0, 8) {

        int col = 0;
        while (*cursor != '/' && *cursor != ' ') {
            switch (*cursor) {
            case '1': col += 1; break;
            case '2': col += 2; break;
            case '3': col += 3; break;
            case '4': col += 4; break;
            case '5': col += 5; break;
            case '6': col += 6; break;
            case '7': col += 7; break;
            case '8': col += 8; break;
            
            case 'p': at(row, col) = BLACK | PAWN;   col += 1; break;
            case 'r': at(row, col) = BLACK | ROOK;   col += 1; break;
            case 'n': at(row, col) = BLACK | KNIGHT; col += 1; break;
            case 'b': at(row, col) = BLACK | BISHOP; col += 1; break;
            case 'q': at(row, col) = BLACK | QUEEN;  col += 1; break;
            case 'k': at(row, col) = BLACK | KING;   col += 1; break;

            case 'P': at(row, col) = WHITE | PAWN;   col += 1; break;
            case 'R': at(row, col) = WHITE | ROOK;   col += 1; break;
            case 'N': at(row, col) = WHITE | KNIGHT; col += 1; break;
            case 'B': at(row, col) = WHITE | BISHOP; col += 1; break;
            case 'Q': at(row, col) = WHITE | QUEEN;  col += 1; break;
            case 'K': at(row, col) = WHITE | KING;   col += 1; break;

            default:
                printf("%c <-\n", *cursor);
                CRASH("");
                break;
            }
            cursor++;
        }
        cursor++;
    }
}

static char* highlight_spectrum[] = {
    "",
    STYLE_BG_Cyan,
    STYLE_BG_Blue,
    STYLE_BG_Green,
    STYLE_BG_Yellow,
    STYLE_BG_Red,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
    STYLE_BG_Magenta,
};

void print_board_debug(Board* b) {
    for_urange(row, 0, 8) {
        for_urange(col, 0, 8) {
            printf("%8b ", at(row, col));
        }
        printf("\n");
    }
}

void print_board_w_moveset(Board* b, MoveSet* ms) {
    u8 highlights[64] = {0};
    foreach (Move m, *ms) highlights[m.target] += 1;
    print_board(b, highlights);
}

void print_board(Board* b, u8* highlights) {
    // printf("    a b c d e f g h  \n");
    // printf("  ┌─────────────────┐\n");
    // printf("  ┌─a─b─c─d─e─f─g─h─┐\n");
    printf("  a b c d e f g h \n");
    for_urange(row, 0, 8) {
        // printf("%d | ", 8 - row);
        printf("%d ", 8 - row);
        
        for_urange(col, 0, 8) {
            bool highlighted = (highlights != NULL && highlights[row * 8 + col]);
            if (highlighted) {
                printf("%s" STYLE_Bold STYLE_FG_Black, highlight_spectrum[highlights[row * 8 + col]]);

                // continue;
            }

            // u8 square_color = ((row + col) % 2 == 0) ? WHITE : BLACK;
            u8 piece_color = piece_color(at(row, col));

            switch (piece_type(at(row, col))) {
            default:
                printf("?");
                break;
            case EMPTY:
                // printf(" ");
                if ((row + col) % 2 == 1) printf(STYLE_Dim);
                printf(".");
                // if ((row + col) % 2 == 1) printf("░");
                // else printf("▒");

                break;
            case PAWN:
                if (piece_color == WHITE)  printf("♟︎");
                else                       printf("♙");
                break;
            case ROOK:
                if (piece_color == WHITE)  printf("♜");
                else                       printf("♖");
                break;
            case BISHOP:
                if (piece_color == WHITE)  printf("♝");
                else                       printf("♗");
                break;
            case KNIGHT:
                if (piece_color == WHITE)  printf("♞");
                else                       printf("♘");
                break;
            case KING:
                if (piece_color == WHITE)  printf("♚");
                else                       printf("♔");
                break;
            case QUEEN:
                if (piece_color == WHITE)  printf("♛");
                else                       printf("♕");
                break;
            }
            printf(" ");
            printf(STYLE_Reset);
        }
        // printf("│ %d\n", 8 - row);
        printf("%d\n", 8 - row);
    }
    printf("  a b c d e f g h\n");
    // printf("  └─a─b─c─d─e─f─g─h─┘\n");
    // printf("  └─────────────────┘\n");
    // printf("    a b c d e f g h  \n");
}