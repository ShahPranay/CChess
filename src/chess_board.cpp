#include<iostream>
#include<math.h>
#include<map>
#include "../headers/chess_board.h"
using namespace std;

void chess_board::debug(){
    for(int i = n_white_pieces; i < n_empty; i++)
        bitboards[i] = 0;
    bitboards[n_empty] = 0xFFFFFFFFFFFFFDFF; //11111101 
    bitboards[n_black_pawn] = 0x0000000000000200;//00000010
    bitboards[n_black_pieces] = 0x0000000000000200;
    draw_bitboard(bitboards[n_black_pawn]);cout<<"\n";
    vector<move_description> moves;
    generate_moves(1,moves);
    for(auto mv: moves){
        draw_bitboard(mv.from | mv.to);cout<<"\n";
        make_move(mv);
        draw_bitboard(bitboards[n_black_knight]);cout<<"\n";
        mailbox_move_update(mv);
        draw_chessboard();
        unmake_move(mv);
        if(mv.is_pawn_promotion)
        cout<<mv.promoted_piece<<"\n";
    }
}

void chess_board::play_chess(){
    int side = 0;
    bool keepgoing = true;
    while(keepgoing){
        move_description mv = get_best_move(side);
        make_move(mv);
        mailbox_move_update(mv);
        side^=1;
        draw_chessboard();
        int tmp;cin>>tmp;
        int boardno;
        if(tmp == 2){
            cin>>boardno;
            while(boardno > 0)
            {draw_bitboard(bitboards[boardno - 1]);cout<<"\n";cin>>boardno;}
        }
        if(tmp == 0)
            keepgoing = false;
    }
}

move_description chess_board::get_best_move(int side){
    pair<int,move_description> bmv;
    bmv = negamax(-INF,INF,6,side);
    return bmv.second;
}

pair<int, move_description> chess_board::negamax(int alpha, int beta, int depthleft, int side){
    if(depthleft == 0){
        move_description tmp{};
        int tmpscore = board_score * (side ? -1 : 1); 
        return {tmpscore,tmp};
    }       
    vector<move_description> list_moves;
    generate_moves(side,list_moves);
    move_description best_move;
    for(auto mv:list_moves){
        make_move(mv);
        auto p = negamax(-beta,-alpha,depthleft - 1, side^1);
        /* cout<<p.first<<" "<<mv.moving_piece<<" "<<log2(mv.from)<<"\n"; */
        unmake_move(mv);
        /* cout<<board_score<<"\n"; */
        p.first *= -1;
        if(p.first >= beta){
            /* cout<<"*\n"; */
            return {beta,best_move};
        }
        if(p.first > alpha){
            alpha = p.first;
            /* cout<<alpha<<"\n"; */
            best_move = mv;
        }
    }
    return {alpha , best_move};
}

U64 chess_board::get_pawn_movebb(U64 frombb, int side){
    if(side == 0)
        return frombb << 8;
    else 
        return frombb >> 8;
}

void chess_board::generate_moves(int side, vector<move_description> &list_moves){
    vector<move_description> quiet_moves; 
    
    // pawn moves 
    U64 bb = bitboards[n_white_pawn + side];

    move_description mv;
    mv.moving_side = side;
    mv.moving_piece = n_white_pawn + side;
    mv.is_capture = false;
    mv.is_pawn_promotion = false;

    while(bb){
        mv.from = bb & -bb;
        U64 to = get_pawn_movebb(mv.from,side);
        if(to & bitboards[n_empty]){
            mv.to = to;
            if((side && (to & rank_1)) || ((!side) && (to & rank_8))){
                mv.is_pawn_promotion = true;
                mv.promoted_piece = n_white_queen + side;
                list_moves.push_back(mv);
                mv.promoted_piece = n_white_knight + side;
                list_moves.push_back(mv);
                mv.is_pawn_promotion = false;
            }
            else
                quiet_moves.push_back(mv);             

            // two steps
            to = get_pawn_movebb(to,side);
            if(to & bitboards[n_empty]){
                if((side && (mv.from & rank_7)) || ((!side) && (mv.from & rank_2))){
                    mv.to = to;
                    quiet_moves.push_back(mv);
                }
            }
        }
        bb ^= mv.from;
    }

    for(int pc = n_white_pawn; pc <= n_white_king; pc +=2){
        bb = bitboards[pc+side];
        while(bb){
            move_description mv;
            U64 from = bb & -bb;
            U64 positions;
            int sqfrom = log2(from);

            if(pc == n_white_knight || pc == n_white_king)
                positions = arr_piece_attacks[pc / 2 - 2][sqfrom] & (~bitboards[side]); 
            else if(pc == n_white_pawn){
                positions = arr_pawn_attacks[side][int(log2(from))];
            }
            else{
                int ind_att_pc = pc / 2 - 2;
                positions = arr_piece_attacks[ind_att_pc][sqfrom];   
                for(U64 bbblockers = ((~bitboards[n_empty]) & arr_blockersandbeyond[ind_att_pc][sqfrom]);bbblockers != 0;){
                    U64 blkr = bbblockers & -bbblockers;
                    positions &= (~arr_behind[sqfrom][int(log2(blkr))]);
                    bbblockers ^= blkr;
                }
                positions &= (~bitboards[side]);
            }
            mv.from = from;
            mv.moving_side = side;
            mv.moving_piece = pc + side;
            mv.is_capture = true;
            mv.is_pawn_promotion = false;

            for(int i = n_white_king; i >= n_white_pawn && positions!=0ULL; i-=2){
                U64 captures = positions & bitboards[i + 1 - side];
                mv.captured_piece = i + 1 - side;
                positions ^= captures;
                while(captures){
                    mv.to = captures & -captures;
                    list_moves.push_back(mv);
                    captures ^= mv.to;
                }
            } 
            mv.is_capture = false;
            while(positions && pc!=n_white_pawn){
                mv.to = positions & -positions;
                quiet_moves.push_back(mv);
                positions ^= mv.to; 
            }
            bb ^= from;
        }
    }

    for(auto m:quiet_moves)
        list_moves.push_back(m);
}

void chess_board::make_move(move_description move){
    U64 change = move.from | move.to;
    int sqfrom = ffsl(move.from) - 1, sqto = ffsl(move.to) - 1;
    int ind_sq_table = (move.moving_piece - move.moving_side)/2 - 1;
    if(move.is_pawn_promotion){
        bitboards[move.moving_piece] ^= move.from;
        bitboards[move.promoted_piece] ^= move.to;
        bitboards[move.moving_side] ^= change;
        bitboards[n_empty] ^= change;
        int ind_table_ppiece = (move.promoted_piece - move.moving_side)/2 - 1;
        int ind_piece_val = (move.moving_piece - move.moving_side)/2 - 1;
        int ind_ppiece_val = (move.promoted_piece - move.moving_side)/2 - 1;
         
        if(move.moving_side == 0){
            board_score += piece_values[ind_ppiece_val] - piece_values[ind_piece_val];
            board_score -= piece_square_tables[ind_sq_table][sqfrom];
            board_score += piece_square_tables[ind_table_ppiece][sqto];
        }
        else{
            sqfrom = 63 - sqfrom;     
            sqto = 63 - sqto;
            board_score -= piece_values[ind_ppiece_val] - piece_values[ind_piece_val];
            board_score += piece_square_tables[ind_sq_table][sqfrom];
            board_score -= piece_square_tables[ind_table_ppiece][sqto];
        }
        return;
    }

    bitboards[move.moving_piece]^=change;
    bitboards[move.moving_side]^=change;
    if(move.moving_side==0){
        board_score += piece_square_tables[ind_sq_table][sqto] - piece_square_tables[ind_sq_table][sqfrom];  
    }
    else{
        sqfrom = 63 - sqfrom; 
        sqto = 63 - sqto;
        board_score -= piece_square_tables[ind_sq_table][sqto] - piece_square_tables[ind_sq_table][sqfrom];
    }
    if(move.is_capture){
        bitboards[move.captured_piece]^=move.to;
        bitboards[1-move.moving_side]^=move.to;
        bitboards[n_empty] ^= move.from;
        int ind_piece_val = (move.captured_piece - 1 + move.moving_side)/2 - 1;
        if(move.moving_side == 1)
            board_score -= piece_values[ind_piece_val];
        else 
            board_score += piece_values[ind_piece_val];
    }
    else 
        bitboards[n_empty]^=change;
}

void chess_board::unmake_move(move_description move){

    U64 change = move.from | move.to;
    int sqfrom = ffsl(move.from) - 1, sqto = ffsl(move.to) - 1;
    int ind_sq_table = (move.moving_piece - move.moving_side)/2 - 1;
    if(move.is_pawn_promotion){
        bitboards[move.moving_piece] ^= move.from;
        bitboards[move.promoted_piece] ^= move.to;
        bitboards[move.moving_side] ^= change;
        bitboards[n_empty] ^= change;
        int ind_table_ppiece = (move.promoted_piece - move.moving_side)/2 - 1;
        int ind_piece_val = (move.moving_piece - move.moving_side)/2 - 1;
        int ind_ppiece_val = (move.promoted_piece - move.moving_side)/2 - 1;
         
        if(move.moving_side == 0){
            board_score -= piece_values[ind_ppiece_val] - piece_values[ind_piece_val];
            board_score += piece_square_tables[ind_sq_table][sqfrom];
            board_score -= piece_square_tables[ind_table_ppiece][sqto];
        }
        else{
            sqfrom = 63 - sqfrom;     
            sqto = 63 - sqto;
            board_score += piece_values[ind_ppiece_val] - piece_values[ind_piece_val];
            board_score -= piece_square_tables[ind_sq_table][sqfrom];
            board_score += piece_square_tables[ind_table_ppiece][sqto];
        }
        return;
    }

    bitboards[move.moving_piece]^=change;
    bitboards[move.moving_side]^=change;
    if(move.moving_side==0){
        board_score -= piece_square_tables[ind_sq_table][sqto] - piece_square_tables[ind_sq_table][sqfrom];  
    }
    else{
        sqfrom = 63 - sqfrom; 
        sqto = 63 - sqto;
        board_score += piece_square_tables[ind_sq_table][sqto] - piece_square_tables[ind_sq_table][sqfrom];
    }
    if(move.is_capture){
        bitboards[move.captured_piece]^=move.to;
        bitboards[1 - move.moving_side]^=move.to;
        bitboards[n_empty]^=move.from;
        int ind_piece_val = (move.captured_piece - 1 + move.moving_side)/2 - 1;
        if(move.moving_side == 1)
            board_score += piece_values[ind_piece_val];
        else 
            board_score -= piece_values[ind_piece_val];
    }
    else 
        bitboards[n_empty]^=change;
}


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
        {'r','n','b','q','k','b','n','r'},
        {'p','p','p','p','p','p','p','p'},
        {'E','E','E','E','E','E','E','E'},
        {'E','E','E','E','E','E','E','E'},
        {'E','E','E','E','E','E','E','E'},
        {'E','E','E','E','E','E','E','E'},
        {'P','P','P','P','P','P','P','P'},
        {'R','N','B','Q','K','B','N','R'}
    };

    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            mailbox[i][j]=tmp[i][j];
        }
    }

    initialise_pieceattacksarr();    
    initialise_score();
}

void chess_board::initialise_score(){
    board_score = 0;
}

void chess_board::mailbox_move_update(move_description bmv){
    map<int,char> symbols{{2,'p'},{3,'P'},{4,'n'},{5,'N'},{6,'b'},{7,'B'},{8,'r'},{9,'R'},{10,'q'},{11,'Q'},{12,'k'},{13,'K'}};
    int sqfrom = log2(bmv.from),sqto = log2(bmv.to);
    int finalpiece;
    if(bmv.is_pawn_promotion)
        finalpiece = bmv.promoted_piece;
    else 
        finalpiece = bmv.moving_piece;
    mailbox[sqfrom >> 3][sqfrom & 7] = 'E';
    mailbox[sqto >> 3][sqto & 7] = symbols[finalpiece];
}

void chess_board::draw_chessboard(){
    cout<<"  ";
    for(int i=0;i<8;i++)
        cout<<char(97+i)<<" ";
    cout<<"\n";
    for(int i=7;i>=0;i--){
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

