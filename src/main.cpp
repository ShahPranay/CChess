#include<iostream>
#include "../headers/chess_board.h"

int main(){
    chess_board Board;
    Board.draw_chessboard();
    /* Board.debug(); */
    Board.play_chess();
    return 0;
}
