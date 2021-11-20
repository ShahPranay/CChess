#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include <vector>
#define ffsl __builtin_ffsl
using namespace std;
typedef unsigned long long U64;

const U64 not_a_file  = 0xFEFEFEFEFEFEFEFE; //11111110
const U64 not_h_file  = 0x7F7F7F7F7F7F7F7F; //01111111 
const U64 not_ab_file = 0xFCFCFCFCFCFCFCFC; //11111100
const U64 not_gh_file = 0x3F3F3F3F3F3F3F3F; // 00111111
const U64 not_8_rank  = 0x00FFFFFFFFFFFFFF;
const U64 not_1_rank  = 0xFFFFFFFFFFFFFF00; 
const U64 rank_2      = 0x000000000000FF00;
const U64 rank_7      = 0x00FF000000000000;
const U64 rank_1      = 0x00000000000000FF;
const U64 rank_8      = 0xFF00000000000000;
const long long INF = 1e9;

enum enum_piece_bb{
    n_white_pieces,
    n_black_pieces,
    n_white_pawn,
    n_black_pawn,
    n_white_knight,
    n_black_knight,
    n_white_bishop,
    n_black_bishop,
    n_white_rook,
    n_black_rook,
    n_white_queen,
    n_black_queen,
    n_white_king,
    n_black_king,
    n_empty
}; // n_white_(piece) + n_black_pieces = n_black_(piece)

enum enum_attack_bb{
    n_attack_knight,
    n_attack_bishop,
    n_attack_rook,
    n_attack_queen,
    n_attack_king
};

enum enum_piece_values{
    n_value_pawn,
    n_value_knight,
    n_value_bishop,
    n_value_rook,
    n_value_queen,
    n_value_king
};

class move_description{
    public:
        U64 from,to;
        int moving_piece,captured_piece,moving_side; 
        bool is_capture, is_pawn_promotion;
        int promoted_piece;
};

class chess_board{
    
    private:
        char mailbox[8][8]; 
        U64 bitboards[15];
        U64 arr_pawn_attacks[2][64];
        U64 arr_piece_attacks[5][64];
        U64 arr_blockersandbeyond[5][64];
        U64 arr_behind[64][64]{};

        int piece_values[6] = {100,320,330,500,900,20000};
        int piece_square_tables[6][64] = {
            {0, 0, 0, 0, 0, 0, 0, 0, 5, 10, 10, -20, -20, 10, 10, 5, 5, -5, -10, 0, 0, -10, -5, 5, 0, 0, 0, 20, 20, 0, 0, 0, 5, 5, 10, 25, 25, 10, 5, 5, 10, 10, 20, 30, 30, 20, 10, 10, 50, 50, 50, 50, 50, 50, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0},
            {-50,-40,-30,-30,-30,-30,-40,-50,-40,-20,  0,  5,  5,  0,-20,-40,-30,  5, 10, 15, 15, 10,  5,-30,-30,  0, 15, 20, 20, 15,  0,-30,-30,  5, 15, 20, 20, 15,  5,-30,-30,  0, 10, 15, 15, 10,  0,-30,-40,-20,  0,  0,  0,  0,-20,-40,-50,-40,-30,-30,-30,-30,-40,-50},
            {-20,-10,-10,-10,-10,-10,-10,-20,-10,  5,  0,  0,  0,  0,  5,-10,-10, 10, 10, 10, 10, 10, 10,-10,-10,  0, 10, 10, 10, 10,  0,-10,-10,  5,  5, 10, 10,  5,  5,-10,-10,  0,  5, 10, 10,  5,  0,-10,-10,  0,  0,  0,  0,  0,  0,-10,-20,-10,-10,-10,-10,-10,-10,-20},
            {0,  0,  0,  5,  5,  0,  0,  0,-5,  0,  0,  0,  0,  0,  0, -5,-5,  0,  0,  0,  0,  0,  0, -5,-5,  0,  0,  0,  0,  0,  0, -5,-5,  0,  0,  0,  0,  0,  0, -5,-5,  0,  0,  0,  0,  0,  0, -5,5, 10, 10, 10, 10, 10, 10,  5,0,  0,  0,  0,  0,  0,  0,  0,},
            {-20,-10,-10, -5, -5,-10,-10,-20,-10,  0,  5,  0,  0,  0,  0,-10,-10,  5,  5,  5,  5,  5,  0,-10,  0,  0,  5,  5,  5,  5,  0, -5, -5,  0,  5,  5,  5,  5,  0, -5,-10,  0,  5,  5,  5,  5,  0,-10,-10,  0,  0,  0,  0,  0,  0,-10,-20,-10,-10, -5, -5,-10,-10,-20},
            {20, 30, 10,  0,  0, 10, 30, 20,20, 20,  0,  0,  0,  0, 20, 20,-10,-20,-20,-20,-20,-20,-20,-10,-20,-30,-30,-40,-40,-30,-30,-20,-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30}
        };
        long long board_score;

        // square index 0 => least significant bit(2^0)
        // square index = 8 * rankindex + fileindex
        // fileindex = squareindex & 7
        // rankindex = squareindex >> 3
        // (6 piece types)*(2 colors) + two color bitboards + 1 empty bitboard
    public:

        chess_board();

        void draw_bitboard(U64 bb);
        void draw_chessboard();
        int evaluate();

        void initialise_pieceattacksarr();
        void initialise_score();

        void make_move(move_description move);
        void unmake_move(move_description move);

        U64 get_pawn_movebb(U64 frombb, int side);
        void generate_moves(int side, vector<move_description> &list_moves);

        move_description get_best_move(int side);
        pair<int,move_description> negamax(int alpha, int beta, int depth, int side);


        void mailbox_move_update(move_description bmv);
        void play_chess();

        void debug();
};

#endif
