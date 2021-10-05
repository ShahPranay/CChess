#include<iostream>
#include<math.h>
#include "../headers/chess_board.h"
using namespace std;

chess_board::chess_board(){
    bitboards[n_empty]=0x0000FFFFFFFF0000;
    bitboards[n_white_pieces]=0x000000000000FFFF;
    bitboards[n_black_pieces]=0xFFFF000000000000;
    bitboards[n_white_pawn]=0x000000000000FF00;
    bitboards[n_black_pawn]=0x00FF000000000000;
    bitboards[n_white_knight]=0x0000000000000042;//01000010
    bitboards[n_black_knight]=0x4200000000000000;
    bitboards[n_white_bishop]=0x0000000000000024;//00100100
    bitboards[n_black_bishop]=0x2400000000000000;
    bitboards[n_white_rook]=0x0000000000000081;//10000001
    bitboards[n_black_rook]=0x8100000000000000;
    bitboards[n_white_queen]=0x0000000000000008;//00010000 this is reversed and represented in the bit board
    bitboards[n_black_queen]=0x0800000000000000;
    bitboards[n_white_king]=0x0000000000000010;//00001000
    bitboards[n_black_king]=0x1000000000000000;

    char tmp[8][8] = {
        {'R','N','B','Q','K','B','N','R'},
        {'P','P','P','P','P','P','P','P'},
        {'E','E','E','E','E','E','E','E'},
        {'E','E','E','E','E','E','E','E'},
        {'E','E','E','E','E','E','E','E'},
        {'E','E','E','E','E','E','E','E'},
        {'p','p','p','p','p','p','p','p'},
        {'r','n','b','q','k','b','n','r'}
    };

    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            mailbox[i][j]=tmp[i][j];
        }
    }

    initialise_pieceattacksarr();    
}

void chess_board::debug(){
}

void chess_board::draw_chessboard(){
    cout<<"  ";
    for(int i=0;i<8;i++)
        cout<<char(97+i)<<" ";
    cout<<"\n";
    for(int i=0;i<8;i++){
        cout<<8-i<<" ";
        for(int j=0;j<8;j++){
            if(mailbox[i][j]=='R')
                cout<<"\u265c ";
            else if(mailbox[i][j]=='N')
                cout<<"\u265e ";
            else if(mailbox[i][j]=='B')
                cout<<"\u265d ";
            else if(mailbox[i][j]=='Q')
                cout<<"\u265b ";
            else if(mailbox[i][j]=='K')
                cout<<"\u265a ";
            else if(mailbox[i][j]=='P')
                cout<<"\u265f ";
            else if(mailbox[i][j]=='r')
                cout<<"\u2656 ";
            else if(mailbox[i][j]=='n')
                cout<<"\u2658 ";
            else if(mailbox[i][j]=='b')
                cout<<"\u2657 ";
            else if(mailbox[i][j]=='q')
                cout<<"\u2655 ";
            else if(mailbox[i][j]=='k')
                cout<<"\u2654 ";
            else if(mailbox[i][j]=='p')
                cout<<"\u2659 ";
            else cout<<"  ";
        }
        cout<<8-i<<" \n";
    }
    cout<<"  ";
    for(int i=0;i<8;i++)
        cout<<char(97+i)<<" ";
    cout<<"\n";
}

void chess_board::draw_bitboard(U64 bb){
    int draw[8][8]{};
    while(bb){
        U64 tmp=bb&-bb;
        int sq=log2(tmp);
        draw[sq>>3][sq&7]=1;
        bb^=tmp;
    }
    for(int i=7;i>=0;i--){
        for(int j=0;j<8;j++){
            cout<<draw[i][j]<<" ";
        }
        cout<<"\n";
    }
}

void chess_board::initialise_pieceattacksarr(){
    // pawn attacks 
    U64 bb=1;
    for(int i=0;i<64;i++){
        arr_pawn_attacks[0][i]=(bb<<7) & not_h_file;
        arr_pawn_attacks[1][i]=(bb>>7) & not_a_file;
        arr_pawn_attacks[0][i]|=(bb<<9) & not_a_file;
        arr_pawn_attacks[1][i]|=(bb>>9) & not_h_file;
        bb<<=1;
    }

    // knight attacks
    bb = 1;
    for(int i=0;i<64;i++){
        arr_piece_attacks[n_attack_knight][i]  = (bb<<15) & not_h_file;
        arr_piece_attacks[n_attack_knight][i] |= (bb<<17) & not_a_file;
        arr_piece_attacks[n_attack_knight][i] |= (bb>>17) & not_h_file;
        arr_piece_attacks[n_attack_knight][i] |= (bb>>15) & not_a_file;
        arr_piece_attacks[n_attack_knight][i] |= (bb<<6)  & not_gh_file; 
        arr_piece_attacks[n_attack_knight][i] |= (bb<<10) & not_ab_file;
        arr_piece_attacks[n_attack_knight][i] |= (bb>>6)  & not_ab_file;
        arr_piece_attacks[n_attack_knight][i] |= (bb>>10) & not_gh_file;
        bb<<=1;
    } 

    // king attacks 
    bb=1;
    for(int i=0;i<64;i++){
        arr_piece_attacks[n_attack_king][i]  = (bb<<8)|(bb>>8);
        arr_piece_attacks[n_attack_king][i] |= (bb<<1) & not_a_file;
        arr_piece_attacks[n_attack_king][i] |= (bb>>1) & not_h_file;
        arr_piece_attacks[n_attack_king][i] |= (bb<<7) & not_h_file;
        arr_piece_attacks[n_attack_king][i] |= (bb>>7) & not_a_file;
        arr_piece_attacks[n_attack_king][i] |= (bb<<9) & not_a_file;
        arr_piece_attacks[n_attack_king][i] |= (bb>>9) & not_h_file;
        bb<<=1;
    }     

    // rook attacks 
    bb=1;
    U64 bbh,bbv; 
    bbh = 0x00000000000000FF;
    for(int i=0;i<8;i++){
        bbv = 0x0101010101010101;//00000001
        for(int j=0;j<8;j++){
            arr_piece_attacks[n_attack_rook][8*i+j] = ((bbh)|(bbv))^bb; 
            bbv<<=1;
            bb<<=1;
        }
        bbh<<=8;
    }

    // bishop attacks
    bbh = 0x8040201008040201;//1000000 
    while(bbh){
        bb = bbh & -bbh;
        int id=log2(bb);
        while(bb){
            arr_piece_attacks[n_attack_bishop][id]=bbh; 
            bb<<=9;
            id+=9;
        }
        bbh<<=8;
    }
    bbh = 0x8040201008040201;//1000000 
    bbh>>=8;
    while(bbh){
        bb = bbh & -bbh;
        int id=log2(bb);
        while(bb){
            arr_piece_attacks[n_attack_bishop][id]=bbh;
            bb = (bb<<9) & not_a_file;
            id+=9;
        }
        bbh>>=8;
    }
    bbh = 0x0102040810204080;//00000001
    while(bbh){
        bb = bbh & -bbh;
        int id=log2(bb);
        while(bb){
            arr_piece_attacks[n_attack_bishop][id]|=bbh;
            arr_piece_attacks[n_attack_bishop][id]^=bb;
            if(id%8==0)
                break;
            bb<<=7;
            id+=7;
        }
        bbh<<=8;
    }
    bbh = 0x0102040810204080;//00000001
    bbh>>=8;
    while(bbh){
        bb = bbh & -bbh;
        int id=log2(bb);
        while(bb){
            arr_piece_attacks[n_attack_bishop][id]|=bbh;
            arr_piece_attacks[n_attack_bishop][id]^=bb;
            if(id%8==0)
                break;
            bb<<=7;
            id+=7;
        }
        bbh>>=8;
    } 

    //queen attacks 
    for(int i=0;i<64;i++){
        arr_piece_attacks[n_attack_queen][i]=arr_piece_attacks[n_attack_rook][i]|arr_piece_attacks[n_attack_bishop][i];
    } 

    // arr behind generation 
    for(int i=0;i<64;i++){
        U64 jsq=1;
        for(int j=0;j<64;j++,jsq<<=1){
            if(j==i)
                continue;
            U64 tmp=0;
            if(arr_piece_attacks[n_attack_queen][i]&jsq){
                int xj=j/8,yj=j%8,xi=i/8,yi=i%8; // x => rank y=> file
                if(xj>xi){
                    if(yj>yi){
                        bb = (jsq<<9)&not_a_file;
                        while(bb){
                            tmp|=bb;
                            bb=((bb<<9)&not_a_file);
                        }
                    }
                    else if(yi==yj){
                        bb=((jsq)<<8);
                        while(bb){
                            tmp|=bb;
                            bb=(bb<<8);
                        }
                    }
                    else{
                        bb = ((jsq)<<7)&not_h_file;
                        while(bb){
                            tmp|=bb;
                            bb=(bb<<7)&not_h_file;
                        }
                    }
                }
                else if(xi==xj){
                    if(yj>yi){
                        bb = ((jsq)<<1)&not_a_file;
                        while(bb){
                            tmp|=bb;
                            bb=(bb<<1)&not_a_file;
                        }
                    }
                    else{
                        bb = ((jsq)>>1)&not_h_file;
                        while(bb){
                            tmp|=bb;
                            bb = (bb>>1)&not_h_file;
                        }
                    }
                }
                else{
                    if(yj>yi){
                        bb = ((jsq)>>7)&not_a_file;
                        while(bb){
                            tmp|=bb;
                            bb=(bb>>7)&not_a_file;
                        }
                    }
                    else if(yj==yi){
                        bb = (jsq)>>8;
                        while(bb){
                            tmp|=bb;
                            bb>>=8;
                        }
                    }
                    else{
                        bb = ((jsq)>>9)&not_h_file;
                        while(bb){
                            tmp|=bb;
                            bb = (bb>>9)&not_h_file;
                        }
                    }
                }
                arr_behind[i][j]=tmp;
            }
        }
    }

    // blockers and beyond array  
    for(int i=0;i<5;i++){
        for(int j=0;j<64;j++){
            arr_blockersandbeyond[i][j]  =  arr_piece_attacks[i][j]; 
            if(j/8!=0){
                arr_blockersandbeyond[i][j] &= not_1_rank;
            }
            if(j/8!=7)
                arr_blockersandbeyond[i][j] &= not_8_rank;
            if(j%8!=0)
                arr_blockersandbeyond[i][j] &= not_a_file;
            if(j%8!=7)
                arr_blockersandbeyond[i][j] &= not_h_file;
        }
    }
}

