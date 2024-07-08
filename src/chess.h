#pragma once

#include "orbit.h"

enum {
    EMPTY = 0,

    PAWN   = 0b00000001,
    ROOK   = 0b00000010,
    BISHOP = 0b00000011,
    KNIGHT = 0b00000100,
    QUEEN  = 0b00000101,
    KING   = 0b00000110,

    WHITE  = 0b00000000,
    BLACK  = 0b00001000,

    HAS_MOVED = 0b00010000,
};

#define swap_color_to_move(b) (b).color_to_move = (b).color_to_move == WHITE ? BLACK : WHITE

#define piece_color(p) ((p) & 0b01000)
#define piece_type(p)  ((p) & 0b00111)

enum {
    SPECIAL_NONE = 0,
    SPECIAL_PAWN_DOUBLE,
    SPECIAL_EN_PASSANT,
    SPECIAL_KINGSIDE_CASTLE,
    SPECIAL_QUEENSIDE_CASTLE,
    SPECIAL_PROMOTE_QUEEN,
    SPECIAL_PROMOTE_ROOK,
    SPECIAL_PROMOTE_BISHOP,
    SPECIAL_PROMOTE_KNIGHT,
};

typedef struct Move {
    u8 start;
    u8 target;
    u8 special;
} Move;

#define NULL_MOVE ((Move){0, 0, 0})
#define is_move_null(m) ((m).start == 0 && (m).target == 0)

// like a Move but with extra info so it's rewindable.
// the board has a stack of these so you can undo moves.
typedef struct GameTick {
    Move move;
    
    u8 captured; // if EMPTY/0, no capture took place (functions as a bool)
    u8 capture_location;

    bool piece_first_move : 1; // this is a piece's first move. removes HAS_MOVED flag.
    bool white_move : 1;
    // probably some inefficiency here, think about it later
} GameTick;

da_typedef(Move);
typedef da(Move) MoveSet;

da_typedef(GameTick);

typedef struct Board {
    u8 board[64];

    u8 color_to_move;

    da(GameTick) move_stack;

    u64 zobrist;
} Board;

void init_board(Board* b);
void load_board(Board* b, char* fen);
void print_board(Board* b, u8* highlights);
void print_board_debug(Board* b);
void print_board_w_moveset(Board* b, MoveSet* ms);

int pseudo_legal_moves(Board* b, MoveSet* mv, bool only_captures);
int filter_illegal_moves(Board* b, MoveSet* mv);

int legal_moves(Board* b, MoveSet* mv);
int legal_captures(Board* b, MoveSet* mv);

void make_move(Board* b, Move mv);
void undo_move(Board* b);


int indexof(char* s);
extern char* square_names[];

typedef struct Player {
    char* name;
    void (*init)(); // player initialization function, runs before any evaluations or moves are made
    int (*eval)(Board*); // board evaluation function (higher is better)
    Move (*select)(Board*); // move selection function, returns the move it wants to make
} Player;

u64 genrand64_int64();
void init_genrand64(u64 seed);

void init_zobrist();
u64 zobrist_component(u8 piece, u8 position);
u64 zobrist_full_board(Board* b);

enum {
    TT_EXACT,
    TT_LOWER,
    TT_UPPER,
};

typedef struct TransposEntry {
    u64 zobrist;
    int eval;
    u16 depth;
    u8  kind;
} TransposEntry;

typedef struct TransposTable {
    TransposEntry* at;
    u64 len;
} TransposTable;

void ttable_init(TransposTable* tt, u64 len);
TransposEntry* ttable_get(TransposTable* tt, u64 zobrist, int depth, int alpha, int beta);
void ttable_put(TransposTable* tt, u64 zobrist, int eval, u16 depth, u8 kind);

extern const Player player_random;
extern const Player player_first;
extern const Player player_v1;
extern const Player player_v2;
extern const Player player_v3;
extern const Player player_v4;