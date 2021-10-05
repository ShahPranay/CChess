#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include <vector>
using namespace std;
typedef unsigned long long U64;

const U64 not_a_file  = 0xFEFEFEFEFEFEFEFE; //11111110
const U64 not_h_file  = 0x7F7F7F7F7F7F7F7F; //01111111 
const U64 not_ab_file = 0xFCFCFCFCFCFCFCFC; //11111100
const U64 not_gh_file = 0x3F3F3F3F3F3F3F3F; // 00111111
const U64 not_8_rank  = 0x00FFFFFFFFFFFFFF;
const U64 not_1_rank  = 0xFFFFFFFFFFFFFF00; 


class move_piece{
    U64 from,to;
    int piece,color; 
};

class chess_board{
    
    private:
        char mailbox[8][8]; 
        U64 bitboards[15];
        U64 arr_pawn_attacks[2][64];
        U64 arr_piece_attacks[5][64];
        U64 arr_blockersandbeyond[5][64];
        U64 arr_behind[64][64]{};
        // square index 0 => least significant bit(2^0)
        // square index = 8 * rankindex + fileindex
        // fileindex = squareindex & 7
        // rankindex = squareindex >> 3
        // (6 piece types)*(2 colors) + two color bitboards + 1 empty bitboard
    public:
        enum enum_piece_bb{
            n_empty,
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
            n_black_king
        };
        enum enum_attack_bb{
            n_attack_knight,
            n_attack_bishop,
            n_attack_rook,
            n_attack_queen,
            n_attack_king
        };

        chess_board();

        void draw_bitboard(U64 bb);
        void draw_chessboard();
        int get_score();

        void initialise_pieceattacksarr();
        void make_move(move_piece move);

        void debug();
};

#endif
