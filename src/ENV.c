
#include"ENV.h"
#define MIN(X,Y) (X)>(Y)?(Y):(X)
#define MAX(X,Y) (X)>(Y)?(X):(Y)
#define XY2ID(X,Y) ((Y)*8+X)



uchar human_promotion_flag=1;//1 if it`s human doing promotion, 0 for simulation
uchar HumanSelectedPromotion=QUEEN;//the selected promotion by human

//update status flags in gameState
void update_flags(GameState *gameState, int start_pt, int end_pt);

int initial_board[64]={CASTLE_B,KNIGHT_B,BISHOP_B,QUEEN_B,KING_B,BISHOP_B,KNIGHT_B,CASTLE_B,
                        PAWN_B,PAWN_B,PAWN_B,PAWN_B,PAWN_B,PAWN_B,PAWN_B,PAWN_B,
                        BLANK,BLANK,BLANK,BLANK,BLANK,BLANK,BLANK,BLANK,
                        BLANK,BLANK,BLANK,BLANK,BLANK,BLANK,BLANK,BLANK,
                        BLANK,BLANK,BLANK,BLANK,BLANK,BLANK,BLANK,BLANK,
                        BLANK,BLANK,BLANK,BLANK,BLANK,BLANK,BLANK,BLANK,
                        PAWN_W,PAWN_W,PAWN_W,PAWN_W,PAWN_W,PAWN_W,PAWN_W,PAWN_W,
                        CASTLE_W,KNIGHT_W,BISHOP_W,QUEEN_W,KING_W,BISHOP_W,KNIGHT_W,CASTLE_W};

//return a new initialized GameState
GameState env_init()
{
    GameState gameState;
    gameState.playerTurn=WHITE;
    gameState.castling_arr[PLAYER1].Left=gameState.castling_arr[PLAYER1].Right=0;
    gameState.castling_arr[PLAYER2].Left=gameState.castling_arr[PLAYER2].Right=0;
    gameState.moves_vector_cnt=0;
    memcpy(gameState.board,initial_board,sizeof(int)*64);
    gameState.moves_stack=NULL;
    return gameState;
}

//free up the memory of the container
void env_free_container(GameState *gameState)
{
    for(int i=0;i<gameState->moves_vector_cnt;i++)
        vector_free(&(gameState->container[i].legal_moves));
    gameState->moves_vector_cnt=0;
}

void env_free_GameState(GameState *gameState)
{
    env_free_container(gameState);
    stack_free(&gameState->moves_stack);
}

void env_reset_GameState(GameState *gameState)
{
    env_free_container(gameState);
    stack_free(&gameState->moves_stack);
    gameState->playerTurn=WHITE;
    gameState->castling_arr[PLAYER1].Left=gameState->castling_arr[PLAYER1].Right=0;
    gameState->castling_arr[PLAYER2].Left=gameState->castling_arr[PLAYER2].Right=0;
    gameState->moves_vector_cnt=0;
    memcpy(gameState->board,initial_board,sizeof(int)*64);
    gameState->moves_stack=NULL;
}

//play on the gameState by the player
void env_play(GameState *gameState, Player *player, int start_pt, int end_pt)
{
    int s_piece=gameState->board[start_pt];
    int e_piece=gameState->board[end_pt];

    int sx=start_pt%8,sy=start_pt/8;
    int ex=end_pt%8,ey=end_pt/8;
    
    int captured_pos=end_pt;
    int SPECIAL_MOVE=NOSPECIAL;
    //if(((s_piece*==6)||(s_piece*==-6))&&(pow((end_pt-start_pt),2)>1))
    int colorID=MAX(gameState->playerTurn*-1,0);
    int PrevMoveCastlingState=(gameState->castling_arr[colorID].Left<<1)|(gameState->castling_arr[colorID].Right);
    update_flags(gameState, start_pt, end_pt);
    gameState->board[start_pt]=0;
    gameState->board[end_pt]=s_piece;
    if((abs(s_piece)==KING)&&(start_pt%8==4))
    {
        if(end_pt==58)//one of the possible four endpoints of a castling bottom/left
        {
            gameState->board[56]=0;
            gameState->board[59]=CASTLE_W;
        }
        else if(end_pt==62)//castling bottom/right
        {
            gameState->board[63]=0;
            gameState->board[61]=CASTLE_W;
        }
        else if(end_pt==6)//castling top/left
        {
            gameState->board[7]=0;
            gameState->board[5]=CASTLE_B;
        }
        else if(end_pt==2)//castling top/right
        {
            gameState->board[0]=0;
            gameState->board[3]=CASTLE_B;
        }
        SPECIAL_MOVE=CASTLING;
    }

    char str_last_move[STR_NODE_SIZE];
    Move last_move;
    stack_peek(gameState->moves_stack, str_last_move);
    last_move = string2move(str_last_move);
    
    if(abs(last_move.piece)== PAWN && (abs(last_move.start_pt - last_move.end_pt) == 16)
        && (abs(s_piece) == PAWN) && abs(start_pt - last_move.end_pt)== 1)
        {
            gameState->board[last_move.end_pt] = BLANK;
            e_piece=PAWN*gameState->playerTurn*-1;
            captured_pos=last_move.end_pt;
            SPECIAL_MOVE=ENPASSANT;
        }
    
    if(abs(s_piece)==PAWN&&(7-2*ey)*gameState->playerTurn==7&&ex==sx)
    {
        int SelectedPromotion=QUEEN;
        if(player->identity==HUMAN)
        {
            human_promotion_flag=!human_promotion_flag;
            if(human_promotion_flag)
            {
                printf("input the promotion you want\n");
                char ch;
                ch=getchar();
                HumanSelectedPromotion=(int)(ch-'0');
            }
            SelectedPromotion=HumanSelectedPromotion;
        }
        else
        {
            SelectedPromotion=QUEEN;
        }
        
        gameState->board[end_pt]=SelectedPromotion*gameState->playerTurn;
        SPECIAL_MOVE=PROMOTION;
    }
    gameState->playerTurn*=-1;
    Move move={s_piece,start_pt,end_pt,e_piece,captured_pos,SPECIAL_MOVE,PrevMoveCastlingState};
    char str_move[STR_NODE_SIZE];
    memset(str_move,'\0',sizeof(str_move));
    move2string(str_move,&move);
    stack_push(&(gameState->moves_stack),str_move);
}

//play on the gameState by the player
void env_play2(GameState *gameState, int start_pt, int end_pt, int promotion)
{
    int s_piece=gameState->board[start_pt];
    int e_piece=gameState->board[end_pt];

    int sx=start_pt%8,sy=start_pt/8;
    int ex=end_pt%8,ey=end_pt/8;
    
    int captured_pos=end_pt;
    int SPECIAL_MOVE=NOSPECIAL;
    //if(((s_piece*==6)||(s_piece*==-6))&&(pow((end_pt-start_pt),2)>1))
    int colorID=MAX(gameState->playerTurn*-1,0);
    int PrevMoveCastlingState=(gameState->castling_arr[colorID].Left<<1)|(gameState->castling_arr[colorID].Right);
    update_flags(gameState, start_pt, end_pt);
    gameState->board[start_pt]=0;
    gameState->board[end_pt]=s_piece;
    if((abs(s_piece)==KING)&&(start_pt%8==4))
    {
        if(end_pt==58)//one of the possible four endpoints of a castling bottom/left
        {
            gameState->board[56]=0;
            gameState->board[59]=CASTLE_W;
        }
        else if(end_pt==62)//castling bottom/right
        {
            gameState->board[63]=0;
            gameState->board[61]=CASTLE_W;
        }
        else if(end_pt==6)//castling top/left
        {
            gameState->board[7]=0;
            gameState->board[5]=CASTLE_B;
        }
        else if(end_pt==2)//castling top/right
        {
            gameState->board[0]=0;
            gameState->board[3]=CASTLE_B;
        }
        SPECIAL_MOVE=CASTLING;
    }

    char str_last_move[STR_NODE_SIZE];
    Move last_move;
    stack_peek(gameState->moves_stack, str_last_move);
    last_move = string2move(str_last_move);
    
    if(abs(last_move.piece)== PAWN && (abs(last_move.start_pt - last_move.end_pt) == 16)
        && (abs(s_piece) == PAWN) && abs(start_pt - last_move.end_pt)== 1)
        {
            gameState->board[last_move.end_pt] = BLANK;
            e_piece=PAWN*gameState->playerTurn*-1;
            captured_pos=last_move.end_pt;
            SPECIAL_MOVE=ENPASSANT;
        }
    
    if(abs(s_piece)==PAWN&&(7-2*ey)*gameState->playerTurn==7&&ex==sx)
    {
        int SelectedPromotion=QUEEN;
        if(promotion>0&&promotion<6)SelectedPromotion=promotion;
        
        gameState->board[end_pt]=SelectedPromotion*gameState->playerTurn;
        SPECIAL_MOVE=PROMOTION;
    }
    gameState->playerTurn*=-1;
    Move move={s_piece,start_pt,end_pt,e_piece,captured_pos,SPECIAL_MOVE,PrevMoveCastlingState};
    char str_move[STR_NODE_SIZE];
    memset(str_move,'\0',sizeof(str_move));
    move2string(str_move,&move);
    stack_push(&(gameState->moves_stack),str_move);
}

//undo the last move
void env_undo(GameState *gameState)
{
    if(!stack_isEmpty((gameState->moves_stack)))
    {
        char str_last_move[STR_NODE_SIZE];
        
        stack_pop(&(gameState->moves_stack),str_last_move);
        Move last_move=string2move(str_last_move);
        if(last_move.end_pt==18)
        {
            int debug=1;
        }
        gameState->board[last_move.start_pt]=last_move.piece;
        gameState->board[last_move.end_pt]=BLANK;
        gameState->board[last_move.captured_pos]=last_move.captured;
        
        int left_castling_flag=last_move.pre_castling_state>>1;
        int right_castlgin_flag=last_move.pre_castling_state&1;
        
        gameState->playerTurn*=-1;
        int colorID=MAX(gameState->playerTurn*-1,0);
        gameState->castling_arr[colorID].Left=left_castling_flag;
        gameState->castling_arr[colorID].Right=right_castlgin_flag;

        if(last_move.special_move==CASTLING)
        {
            int x=last_move.end_pt%8;
            if(x==6)
            {
                gameState->board[last_move.end_pt+1]=gameState->playerTurn*CASTLE;
                gameState->board[last_move.end_pt-1]=BLANK;
            }
            else if(x==2)
            {
                gameState->board[last_move.end_pt-2]=gameState->playerTurn*CASTLE;
                gameState->board[last_move.end_pt+1]=BLANK;
            }
        }
    }
}

//check whether the location in given vector or the enemy`s king is threatened
//in the current player`s turn
uchar env_is_threatened(GameState *gameState,Player *player, vector *check_slots)
{
    int pos=-1;
    vector legal_moves;
    uchar threatened_area[64];
    memset(threatened_area,0,sizeof(uchar)*64);
    int K=-1;
    for(int y=0;y<8;y++)
    {
        for(int x=0;x<8;x++)
        {
            pos=y*8+x;
            if(gameState->board[pos]*gameState->playerTurn*-1==KING) K=pos;
            else if(gameState->board[pos]*gameState->playerTurn>0)
            {
                if(gameState->board[pos]*gameState->playerTurn==KING)legal_moves=env_get_legal_king(gameState,player,pos,0);
                else legal_moves=env_get_legal_moves(gameState,player,pos);
                for(int i=0;i<legal_moves.count;i++)
                {
                    threatened_area[vector_get(&legal_moves,i)]=1;
                }
                vector_free(&legal_moves);
            }
        }
    }
    if(check_slots->count>0)
    {
        int cnt=check_slots->count;
        for(int i=0;i<cnt;i++)
        {
            pos=vector_get(check_slots,i);
            vector_set(check_slots,i,threatened_area[pos]);
        }
        return NULL;
    }
    else if(threatened_area[K]==1)return 1;
    else return 0;
}

//check if checkmate, 0 for no checking
//1 for lose
//2 for win 
uchar env_check_end(GameState *gameState, Player *player)
{
    vector legal_moves;
    int pos=-1,myK=-1;
    uchar threatened=1;
    uchar end=1;
    vector empty;
    vector_init(&empty);
    for(int y=0;y<8;y++)
    {
        for(int x=0;x<8;x++)
        {
            pos=y*8+x;
            if(gameState->board[pos]*gameState->playerTurn==KING)myK=pos;
            if(gameState->board[pos]*gameState->playerTurn>0)
            {
                legal_moves=env_get_legal_moves(gameState,player,pos);
                if(legal_moves.count>0)
                {
                    gameState->container[gameState->moves_vector_cnt].pos=pos;
                    gameState->container[gameState->moves_vector_cnt].legal_moves=legal_moves;
                    gameState->moves_vector_cnt++;
                }
                else
                    continue;
                for(int i=0;i<legal_moves.count;i++)
                {
                    env_play(gameState,player,pos,vector_get(&legal_moves,i));
                    threatened=env_is_threatened(gameState,player,&empty);
                    env_undo(gameState);
                    if(threatened==0)
                    {
                        end=0;
                        break;
                    }
                    else if(threatened==2)end=2;
                }
                //here is the only position where vector_free is not necessary
            }
        }
    }
    if(myK==-1)end=2;
    return end;
}

//shallow copy the gameState, the move vector and stack will not be copied
GameState env_copy_State(GameState *gameState)
{
    GameState newState;
    memcpy(newState.board,gameState->board,64*sizeof(int));
    newState.playerTurn=gameState->playerTurn;
    newState.castling_arr[0]=gameState->castling_arr[0];
    newState.castling_arr[1]=gameState->castling_arr[1];
    newState.moves_vector_cnt=0;
    return newState;
}

//get all legal moves for different pieces, returns a vector
vector env_get_legal_moves(GameState *gameState, Player *player, int start_pt)
{
    
    vector legal_moves;
    vector_init(&legal_moves);
    if(start_pt<0||start_pt>=64)return legal_moves;
    if(gameState->playerTurn*gameState->board[start_pt]<=0)return legal_moves;
    switch(abs(gameState->board[start_pt]))
    {
        case PAWN:
            legal_moves=env_get_legal_pawn(gameState,start_pt);
            break;
        case KNIGHT:
            legal_moves=env_get_legal_knight(gameState,start_pt);
            break;
        case CASTLE:
            legal_moves=env_get_legal_castle(gameState,start_pt);
            break;
        case BISHOP:
            legal_moves=env_get_legal_bishop(gameState,start_pt);
            break;
        case QUEEN:
            legal_moves=env_get_legal_queen(gameState,start_pt);
            break;
        case KING:
            legal_moves=env_get_legal_king(gameState,player,start_pt,1);
            break;
    }
    return legal_moves;
}

//board[pos]*playerTurn<0 -> enemy
//board[pos]*playerTurn>0 -> mime
//get legal moves of different pieces
vector env_get_legal_pawn(GameState *gameState, int start_pt)
{
    vector legal_moves;
    vector_init(&legal_moves);
    int x=start_pt%8, y=start_pt/8;

    int playerTurn=gameState->playerTurn;
    int maxStep=1;
    if((7-y*2)*playerTurn==-5)//means that pawn is on home row
        maxStep=2;
    for(int k=1;k<=maxStep;k++)
    {
        if(abs(7-(y+k*playerTurn*-1)*2)>7)break;
        if(gameState->board[XY2ID(x,y+k*playerTurn*-1)]*playerTurn==0)vector_add(&legal_moves,XY2ID(x,y+k*playerTurn*-1));
        else break;
    }
    for(int dx=-1;dx<=1;dx+=2)
    {
        if(abs(7-(y+playerTurn*(-1))*2)>7)break;
        if(x+dx<0||x+dx>7)continue;
        if(gameState->board[XY2ID(x+dx,y-playerTurn)]*playerTurn<0)vector_add(&legal_moves,XY2ID(x+dx,y-playerTurn));
    }
    char str_last_move[STR_NODE_SIZE];
    Move last_move;
    stack_peek(gameState->moves_stack, str_last_move);
    last_move = string2move(str_last_move);
    if(abs(last_move.piece )== PAWN && (abs(last_move.start_pt - last_move.end_pt) == 16)
        && abs(start_pt - last_move.end_pt)== 1)
        {
            vector_add(&legal_moves, last_move.start_pt + 8 * gameState->playerTurn);
        }
                                                                   
    return legal_moves;
}

void env_get_legal_cross(GameState *gameState, vector *legal_moves, int start_pt, int step)
{
    int x=start_pt%8, y=start_pt/8;
    int playerTurn=gameState->playerTurn;
    int xMax=MIN(x+step+1,8), xMin=MAX(x-step,0);
    int yMax=MIN(y+step+1,8), yMin=MAX(y-step,0);
    for(int i=y+1;i<yMax;i++)//iterates through the y in one direction
    {
        if(gameState->board[i*8+x]==0)
        {
            vector_add(legal_moves,(i*8+x));
        }
        else if (gameState->board[i*8+x]*playerTurn<0)
        {
            vector_add(legal_moves,(i*8+x));//adds opponents space
            break;//ternimates loop
        }
        else
            break;
    }
                       
    for(int i=y-1;i>=yMin;i--)
    {
        if(gameState->board[i*8+x]==0)
        {
            vector_add(legal_moves,(i*8+x));
        }
        else if (gameState->board[i*8+x]*playerTurn<0)
        {
            vector_add(legal_moves,(i*8+x));//adds opponents space
            break;//ternimates loop
        }
        else
            break;
    }
    
    for(int i=x+1;i<xMax;i++)//iterates through the y in one direction
    {
        if(gameState->board[y*8+i]==0)
        {
            vector_add(legal_moves,(y*8+i));//adds every empty space
        }
        else if (gameState->board[y*8+i]*playerTurn<0)
        {
            vector_add(legal_moves,(y*8+i));//adds opponents space
            break;//ternimates loop
        }
        else
            break;
   }
    for(int i=x-1;i>=xMin;i--)
    {
        if(gameState->board[y*8+i]==0)
        {
            vector_add(legal_moves,(y*8+i));
        }
        else if (gameState->board[y*8+i]*playerTurn<0)
        {
            vector_add(legal_moves,(y*8+i));//adds opponents space
            break;//ternimates loop
        }
        else
            break;
    }

}

void env_get_legal_diagonal(GameState *gameState, vector *legal_moves, int start_pt, int step)
{
    int x=start_pt%8, y=start_pt/8;
    int playerTurn=gameState->playerTurn;
    int xMax=MIN(x+step+1,8), xMin=MAX(x-step,0);
    int yMax=MIN(y+step+1,8), yMin=MAX(y-step,0);
    for(int i=y+1,j=x+1 ; i<yMax && j<xMax ; i++, j++)//iterates through the x,y in one direction
    {
        if(gameState->board[i*8+j]==0)
        {
              vector_add(legal_moves,(i*8+j));//adds every empty space
        }
        else if (gameState->board[i*8+j]*playerTurn<0)
        {
            vector_add(legal_moves,(i*8+j));//adds opponents space
            break;//ternimates loop
        }
        else
        {
            break;//terminates if something blocks its path
        }
    }
    
    for(int i=y+1,j=x-1 ; i<yMax && j>=xMin ; i++, j--)//iterates through the x,y in one direction
    {
        if(gameState->board[i*8+j]==0)
        {
            vector_add(legal_moves,(i*8+j));//adds every empty space
        }
        else if (gameState->board[i*8+j]*playerTurn<0)
        {
            vector_add(legal_moves,(i*8+j));//adds opponents space
            break;//ternimates loop
        }
        else
        {
            break;//terminates if something blocks its path
        }
   }

    for(int i=y-1,j=x-1 ; i>=yMin && j>=xMin ; i--, j--)//iterates through the x,y in one direction
    {
        if(gameState->board[i*8+j]==0)
        {
            vector_add(legal_moves,(i*8+j));//adds every empty space
        }
        else if (gameState->board[i*8+j]*playerTurn<0)
        {
            vector_add(legal_moves,(i*8+j));//adds opponents space
            break;//ternimates loop
        }
        else
        {
            break;//terminates if something blocks its path
        }
    }
    
    for(int i=y-1,j=x+1 ; i>=yMin && j<xMax ; i--, j++)//iterates through the x,y in one direction
    {
        if(gameState->board[i*8+j]==0)
        {
            vector_add(legal_moves,(i*8+j));//adds every empty space
        }
        else if (gameState->board[i*8+j]*playerTurn<0)
        {
            vector_add(legal_moves,(i*8+j));//adds opponents space
            break;//ternimates loop
        }
        else
        {
            break;//terminates if something blocks its path
        }
    }
}

vector env_get_legal_castle(GameState *gameState, int start_pt)
{
    vector legal_moves;
    vector_init(&legal_moves);
    env_get_legal_cross(gameState,&legal_moves,start_pt,8);
    return legal_moves;
}

vector env_get_legal_bishop(GameState *gameState, int start_pt)
{
    vector legal_moves;
    vector_init(&legal_moves);
    env_get_legal_diagonal(gameState,&legal_moves,start_pt,8);
    return legal_moves;
}

vector env_get_legal_queen(GameState *gameState, int start_pt)
{
    vector legal_moves1,legal_moves2;
    vector_init(&legal_moves1);
    vector_init(&legal_moves2);
    env_get_legal_cross(gameState,&legal_moves1,start_pt,8);
    env_get_legal_diagonal(gameState,&legal_moves2,start_pt,8);
    vector_cat(&legal_moves1,&legal_moves2);
    vector_free(&legal_moves2);
    return legal_moves1;
}

//check castling availability, add legal moves to legal_moves vector
void env_get_legal_castling(GameState *gameState,Player *player, vector *legal_moves, int start_pt)
{
    int x=start_pt%8, y=start_pt/8;
    int playerTurn=gameState->playerTurn;
    //space numbers are hardcoded in
    vector check_check_slots;
    vector_init(&check_check_slots);
    for(int i=-3;i<=2;i++)vector_add(&check_check_slots,start_pt+i);
    gameState->playerTurn*=-1;
    env_is_threatened(gameState,player,&check_check_slots);
    gameState->playerTurn*=-1;
    if(vector_get(&check_check_slots,3)==1)
    {
        vector_free(&check_check_slots);
        return;
    }
    if(((y==7)&&(x==4))&&(playerTurn==1))//checks for king being in its original position
    {
        int i=1;//just set it to any other than 0
        
        if(gameState->board[56]==3)
        {
            for(i=-3;i<0;i++)
            {
                if(gameState->castling_arr[0].Left)break;
                if(gameState->board[start_pt+i]==BLANK&&!vector_get(&check_check_slots,i+3))continue;//3 is for the offset for i
                else break;
            }
        }
        if(i==0)vector_add(legal_moves,(58));
        i=1;
        if(gameState->board[63]==3)
        {
            for(i=2;i>0;i--)
            {
                if(gameState->castling_arr[0].Right)break;
                if(gameState->board[start_pt+i]==BLANK&&!vector_get(&check_check_slots,i+3))continue;
                else break;
            }
        }
        if(i==0)vector_add(legal_moves,62);

    }
       
    if(((y==0)&&(x==4))&&(playerTurn==-1))//checks for king being in its original position
    {
        int i=1;
        if(gameState->board[0]==CASTLE_B)
        {
            
            for(i=-3;i<0;i++)
            {
                if(gameState->castling_arr[1].Left)break;
                if(gameState->board[start_pt+i]==BLANK&&!vector_get(&check_check_slots,i+3))continue;//3 is for the offset for i
                else break;
            }
        }
        if(i==0)vector_add(legal_moves,(2));
        i=1;
        if(gameState->board[7]==CASTLE_B)
        {
            for(i=2;i>0;i--)
            {
                if(gameState->castling_arr[1].Right)break;
                if(gameState->board[start_pt+i]==BLANK&&!vector_get(&check_check_slots,i+3))continue;
                else break;
            }
        }
        if(i==0)vector_add(legal_moves,6);
    }
    vector_free(&check_check_slots);
}

//get king`s legal moves, check_castling==1 if 
//try to check castling availability
vector env_get_legal_king(GameState *gameState, Player *player, int start_pt, uchar check_castling)
{
    vector legal_moves1,legal_moves2,legal_moves3;
    vector_init(&legal_moves1);
    vector_init(&legal_moves2);
    vector_init(&legal_moves3);
    env_get_legal_cross(gameState,&legal_moves1,start_pt,1);
    env_get_legal_diagonal(gameState,&legal_moves2,start_pt,1);
    if(check_castling)env_get_legal_castling(gameState,player,&legal_moves3,start_pt);//a third vector for castling
    vector_cat(&legal_moves1,&legal_moves2);
    vector_free(&legal_moves2);
    vector_cat(&legal_moves1,&legal_moves3);
    vector_free(&legal_moves3);
    return legal_moves1;
}

vector env_get_legal_knight(GameState *gameState, int start_pt)
{
    vector legal_moves;
    vector_init(&legal_moves);
    int x=start_pt%8, y=start_pt/8;
    
    //jumps that are +/- 2 in the y
    if((x+1<8 && y+2<8)&&(gameState->board[((y+2)*8+(x+1))]*gameState->board[start_pt]<=0))
    {
        vector_add(&legal_moves,((y+2)*8+(x+1)));
    }
    if((x-1>=0 && y+2<8)&&(gameState->board[((y+2)*8+(x-1))]*gameState->board[start_pt]<=0))
    {
        vector_add(&legal_moves,((y+2)*8+(x-1)));
    }
    if((x+1<8 && y-2>=0)&&(gameState->board[((y-2)*8+(x+1))]*gameState->board[start_pt]<=0))
    {
        vector_add(&legal_moves,((y-2)*8+(x+1)));
    }
    if((x-1>=0 && y-2>=0)&&(gameState->board[((y-2)*8+(x-1))]*gameState->board[start_pt]<=0))
    {
        vector_add(&legal_moves,((y-2)*8+(x-1)));
    }
    
    //jumps that are +/- 2 in the x
    if((x+2<8 && y+1<8)&&gameState->board[((y+1)*8+(x+2))]*gameState->board[start_pt]<=0)
    {
        vector_add(&legal_moves,((y+1)*8+(x+2)));
    }
    if((x-2>=0 && y+1<8)&&gameState->board[((y+1)*8+(x-2))]*gameState->board[start_pt]<=0)
    {
        vector_add(&legal_moves,((y+1)*8+(x-2)));
    }
    if((x+2<8 && y-1>=0)&&gameState->board[((y-1)*8+(x+2))]*gameState->board[start_pt]<=0)
    {
        vector_add(&legal_moves,((y-1)*8+(x+2)));
    }
    if((x-2>=0 && y-1>=0)&&gameState->board[((y-1)*8+(x-2))]*gameState->board[start_pt]<=0)
    {
        vector_add(&legal_moves,((y-1)*8+(x-2)));
    }
    
    return legal_moves;
}

//update status flag in gameState
void update_flags(GameState *gameState, int start_pt, int end_pt)
{
    

    int s_piece=gameState->board[start_pt];
    if (s_piece == KING_W&&start_pt==60)//if white king moves
    {
        gameState->castling_arr[PLAYER1].Left=gameState->castling_arr[PLAYER1].Right=1;//sets both flags down
    }
    
    else if (s_piece == KING_B&&start_pt==4)//if black king moves
    {
      	gameState->castling_arr[PLAYER2].Left=gameState->castling_arr[PLAYER2].Right=1;//sets both flags down
    }
    
    
    if (s_piece == CASTLE_W && start_pt == 56)//if left rook moves from its original spot
    {
        gameState->castling_arr[PLAYER1].Left=1;//lowers left flag
    }
    else if (s_piece == CASTLE_W && start_pt == 63)//if left rook moves from its original spot
    {
        gameState->castling_arr[PLAYER1].Right=1;//lowers Right flag
    }
    else if (s_piece == CASTLE_B && start_pt == 0)//if left rook moves from its original spot
    {
        gameState->castling_arr[PLAYER2].Left=1;//lowers left flag
    }
    else if (s_piece == CASTLE_B && start_pt == 7)//if left rook moves from its original spot
    {
        gameState->castling_arr[PLAYER2].Right=1;//lowers left flag
    }
    
}
