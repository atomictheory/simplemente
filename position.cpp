#include "position.h"
#include "simplemente.h"

#include "xboard.h"

#include "xxhash.h"
#include <iostream>
#include <fstream>
#include <time.h>

#include <string>

using namespace std;

int randh()
{
	return rand() % RANDOM_BONUS;
}

bool quit_search;
bool search_quitted=true;
bool disable_search_quitted=false;
bool do_make_move=false;

Move hash_table[HASH_TABLE_SIZE];

int material_values[MAX_PIECE];

Move* move_table_ptr[BOARD_SIZE][MAX_PIECE];

Move move_table[BOARD_SIZE*MAX_PIECE*MAX_LEGAL_MOVES_PER_SQUARE*2];

void init_material_values()
{
	material_values[WHITE_PIECE|KNIGHT]=KNIGHT_VALUE;
	material_values[WHITE_PIECE|BISHOP]=BISHOP_VALUE;
	material_values[WHITE_PIECE|ROOK]=ROOK_VALUE;
	material_values[WHITE_PIECE|QUEEN]=QUEEN_VALUE;
	material_values[WHITE_PIECE|KING]=0;
	material_values[WHITE_PIECE|PAWN]=PAWN_VALUE;

	material_values[KNIGHT]=-KNIGHT_VALUE;
	material_values[BISHOP]=-BISHOP_VALUE;
	material_values[ROOK]=-ROOK_VALUE;
	material_values[QUEEN]=-QUEEN_VALUE;
	material_values[KING]=0;
	material_values[PAWN]=-PAWN_VALUE;
}

void init_hash_table()
{
	memset(hash_table,SQUARE_NONE,HASH_TABLE_SIZE);
}

void init_move_table()
{

	init_material_values();
	init_hash_table();

	Move* ptr=move_table;

	for(Square sq=0;sq<BOARD_SIZE;sq++)
	{

		Rank rank=RANK_OF(sq);
		File file=FILE_OF(sq);

		for(Color color=BLACK_PIECE;color<=WHITE_PIECE;color+=WHITE_PIECE)
		{
			for(Piece piece=KING;piece<=QUEEN;piece++)
			{

				#ifdef MY_DEBUG_MOVE_GEN
				cout 
					<< "ptr " << (int)(ptr-move_table)
					<< " square " << (int)sq 
					<< " rank " << (int)rank 
					<< " file " << (int)file
					<< " piece " << (int)piece 
					<< " color " << (int)color 
					<< endl;
				#endif

				switch(piece)
				{
					case KING:
					{

						move_table_ptr[sq][color|KING]=ptr;

						for(int i=-1;i<=1;i++)
						{
							for(int j=-1;j<=1;j++)
							{
								int to_file=file+i;
								int to_rank=rank+j;

								if(VALID_FILE(to_file)&&VALID_RANK(to_rank))
								{
									Move move;

									move.from_sq=sq;
									move.to_sq=SQUARE_FROM_FR(to_file,to_rank);

									move.info=SINGLE_MOVE;

									*ptr++=move;

									#ifdef MY_DEBUG_MOVE_GEN
									cout << piece_letters[color|piece];
									move.print();
									#endif
								}
							}
						}

						if((color==WHITE_PIECE)&&(sq==WHITE_KING_CASTLE_FROM_SQUARE))
						{
							Move move;

							move.from_sq=sq;
							move.to_sq=CASTLING_EMPTY_SQUARE_W_K_2;

							move.info=SINGLE_MOVE|CASTLING;

							*ptr++=move;

							move.to_sq=CASTLING_EMPTY_SQUARE_W_Q_2;

							*ptr++=move;
						}

						if((color==BLACK_PIECE)&&(sq==BLACK_KING_CASTLE_FROM_SQUARE))
						{
							Move move;

							move.from_sq=sq;
							move.to_sq=CASTLING_EMPTY_SQUARE_B_K_2;

							move.info=SINGLE_MOVE|CASTLING;

							*ptr++=move;

							move.to_sq=CASTLING_EMPTY_SQUARE_B_Q_2;

							*ptr++=move;
						}

						ptr->info=END_PIECE;
						ptr++;

					} break;

					case KNIGHT:
					{

						move_table_ptr[sq][color|KNIGHT]=ptr;

						for(int i=-2;i<=2;i++)
						{
							for(int j=-2;j<=2;j++)
							{
								if(abs(i*j)==2)
								{
									int to_file=file+i;
									int to_rank=rank+j;

									if(VALID_FILE(to_file)&&VALID_RANK(to_rank))
									{
										Move move;

										move.from_sq=sq;
										move.to_sq=SQUARE_FROM_FR(to_file,to_rank);

										move.info=SINGLE_MOVE;

										*ptr++=move;

										#ifdef MY_DEBUG_MOVE_GEN
										cout << piece_letters[color|piece];
										move.print();
										#endif
									}
								}
							}
						}

						ptr->info=END_PIECE;
						ptr++;

					} break;

					case ROOK:
					case BISHOP:
					case QUEEN:
					{

						move_table_ptr[sq][color|piece]=ptr;

						for(int i=-1;i<=1;i++)
						{
							for(int j=-1;j<=1;j++)
							{
								
								int to_file=file+i;
								int to_rank=rank+j;

								Move* vector_start_ptr=ptr;

								while(
									(VALID_FILE(to_file)&&VALID_RANK(to_rank))
									&&
									((abs(i)+abs(j))>0)
									&&
									(
										((piece & STRAIGHT)&&(i*j==0))
										||
										((piece & DIAGONAL)&&(abs(i*j)==1))
									)
								)
								{
									Move move;

									move.from_sq=sq;
									move.to_sq=SQUARE_FROM_FR(to_file,to_rank);

									move.info=SLIDING_MOVE;

									*ptr++=move;

									#ifdef MY_DEBUG_MOVE_GEN
									cout << piece_letters[color|piece];
									move.print();
									#endif

									to_file=to_file+i;
									to_rank=to_rank+j;
								}

								for(Move* set_ptr=vector_start_ptr;set_ptr<ptr;set_ptr++)
								{
									set_ptr->next_vector=ptr;
								}

							}
						}

						ptr->info=END_PIECE;
						ptr++;

					} break;

					case PAWN:
					{

						move_table_ptr[sq][color|PAWN]=ptr;

						// captures
						for(int i=-1;i<=1;i+=2)
						{
							int to_file=file+i;
							if(VALID_FILE(to_file))
							{
								int to_rank=rank+(color==WHITE_PIECE?-1:1);

								if(VALID_RANK(to_rank))
								{
									Move move;

									move.from_sq=sq;
									move.to_sq=SQUARE_FROM_FR(to_file,to_rank);

									Square ep_sq_info=SQUARE_FROM_FR(to_file,rank);

									move.info=CAPTURE;
									move.info|=ep_sq_info;

									*ptr++=move;
								}
							}
						}

						Square pawn_pass_square;
						// pushes
						for(int i=1;i<=2;i++)
						{
							int to_rank=rank+(color==WHITE_PIECE?-i:i);

							if(
								(VALID_RANK(to_rank))
								&&
								(
									// push by one always possible
									(i==1)
									||
									(
										// push by two only possible in initial position
										(i==2)
										&&
										(rank==(color==WHITE_PIECE?6:1))
									)
								)
							)
							{
								Move move;

								move.from_sq=sq;
								move.to_sq=SQUARE_FROM_FR(file,to_rank);

								move.info=PAWN_PUSH;

								if(i==1)
								{
									pawn_pass_square=move.to_sq;

									if(
										((rank==1)&&(color==WHITE_PIECE))
										||
										((rank==6)&&(color==BLACK_PIECE))
									)
									{
										move.info|=PROMOTION;
									}
								}

								if(i==2)
								{
									move.info|=PAWN_PUSH_BY_TWO;
									move.info|=pawn_pass_square;
								}

								*ptr++=move;
							}
						}

						ptr->info=END_PIECE;
						ptr++;

					} break;

					default:

					move_table_ptr[sq][color|piece]=ptr;

					ptr->info=END_PIECE;
					ptr++;

				}
			}
		}
	}

	#ifdef MY_DEBUG_MOVE_GEN
	cout << "move table size " << sizeof(move_table) << endl;
	#endif
}

void Position::print()
{

	cout << endl;

	for(Square sq=0;sq<BOARD_SIZE;sq++)
	{
		Piece p=board[sq];

		if(FILE_OF(sq)==0)
		{
			cout << (char)(BOARD_HEIGHT-1-RANK_OF(sq)+'1') << "  ";
		}

		cout << (p?piece_letters[p]:'.');
		
		if(FILE_OF(sq)==FILE_MASK)
		{
			cout << endl;
		}
	}

	cout << endl << "   ";

	for(File f=0;f<BOARD_WIDTH;f++)
	{
		cout << (char)(f+'a');
	}

	cout << endl;
	cout << endl;
	cout << "turn " << (turn==WHITE?"white":"black") << " , ";
	cout << "castling rights ["
		<< (castling_rights&CASTLING_RIGHT_K?"K":"")
		<< (castling_rights&CASTLING_RIGHT_Q?"Q":"")
		<< (castling_rights&CASTLING_RIGHT_k?"k":"")
		<< (castling_rights&CASTLING_RIGHT_q?"q":"")
		<< (castling_rights==CASTLING_RIGHT_NONE?"-":"")
	<< "]" << " , ";
	cout << "ep square [" << square_to_algeb(ep_square) << "]" << endl;
	//cout << "material balance " << calc_material_balance() << endl;
	//cout << "mobility balance " << calc_mobility_balance() << endl;
	//cout << "attacker balance " << calc_attacker_balance() << endl;
	//cout << "heuristic value " << calc_heuristic_value() << endl;
	//cout << "is in check white " << (int)is_in_check(WHITE) << endl;
	//cout << "is in check black " << (int)is_in_check(BLACK) << endl;

		#ifdef MY_DEBUG_POSITION_PRINT
		cout << endl;
		cout << "hash size " << (int)sizeof(hash_table) << endl;
		cout << "hash key: " << calc_hash_key();
		cout << endl;
		#endif
	cout << endl;

}


void Position::reset()
{
	set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Position::set_from_fen(char* fen)
{
	Square sq=0;
	char f;

	memset(board,NO_PIECE,BOARD_SIZE);

	king_pos[WHITE]=63;
	king_pos[BLACK]=0;

	init_move_generator();

	// defaults

	turn=WHITE;
	castling_rights=CASTLING_RIGHT_NONE;
	ep_square=SQUARE_NONE;

	halfmove_clock=0;
	fullmove_number=1;

	do
	{
		f=*fen++;
		if((f>='1')&&(f<='9'))
		{
			f-='0';
			while(f--)
			{
				board[sq++]=NO_PIECE;
			}
		}
		else
		{
			switch(f)
			{
				case 'N': board[sq++]=WHITE_PIECE|KNIGHT;break;
				case 'n': board[sq++]=BLACK_PIECE|KNIGHT;break;
				case 'B': board[sq++]=WHITE_PIECE|BISHOP;break;
				case 'b': board[sq++]=BLACK_PIECE|BISHOP;break;
				case 'R': board[sq++]=WHITE_PIECE|ROOK;break;
				case 'r': board[sq++]=BLACK_PIECE|ROOK;break;
				case 'Q': board[sq++]=WHITE_PIECE|QUEEN;break;
				case 'q': board[sq++]=BLACK_PIECE|QUEEN;break;
				case 'K':
						{
							board[sq]=WHITE_PIECE|KING;
							king_pos[WHITE]=sq++;
							break;
						}
				case 'k':
						{
							board[sq]=BLACK_PIECE|KING;
							king_pos[BLACK]=sq++;
							break;
						}
				case 'P': board[sq++]=WHITE_PIECE|PAWN;break;
				case 'p': board[sq++]=BLACK_PIECE|PAWN;break;
			}
		}
	}while(f&&(f!=' ')&&(sq<BOARD_SIZE));

	while(f&&(f!=' ')){f=*fen++;};

	if(f!=' ')
	{
		// invalid fen, use defaults
		return;
	}

	f=*fen++;

	turn=f=='w'?WHITE:BLACK;

	f=*fen++;

	if(f!=' ')
	{
		// invalid fen, use defaults
		return;
	}

	f=*fen++;

	while(f&&(f!=' '))
	{
		switch(f)
		{
			case 'K': castling_rights|=CASTLING_RIGHT_K;break;
			case 'Q': castling_rights|=CASTLING_RIGHT_Q;break;
			case 'k': castling_rights|=CASTLING_RIGHT_k;break;
			case 'q': castling_rights|=CASTLING_RIGHT_q;break;
		}
		f=*fen++;
	}

	if(f!=' ')
	{
		// invalid fen, use defaults
		return;
	}

	f=*fen;

	if(f!='-')
	{
		ep_square=algeb_to_square(fen);
	}

	// ignore halfmove clock

	// ignore fullmove number

}

Square Position::algeb_to_square(char* algeb)
{
	if((algeb[0]<'a')||(algeb[0]>('a'+BOARD_WIDTH-1))){return SQUARE_NONE;}
	if((algeb[1]<'1')||(algeb[1]>('1'+BOARD_HEIGHT-1))){return SQUARE_NONE;}

	return(algeb[0]-'a'+((BOARD_HEIGHT-1)-(algeb[1]-'1'))*BOARD_WIDTH);
}

HashKey Position::calc_hash_key()
{
	hash_key=(HashKey)XXH32((void*)this,sizeof(PositionTrunk),0);

	hash_key&=HASH_TABLE_MASK;

	return hash_key;
}

char sq_to_algeb_puff[3];
char* square_to_algeb(Square sq)
{

	if(sq==SQUARE_NONE)
	{
		sq_to_algeb_puff[0]='-';
		sq_to_algeb_puff[1]=0;
		return sq_to_algeb_puff;
	}

	File f=FILE_OF(sq);
	Rank r=RANK_OF(sq);

	sq_to_algeb_puff[0]=f+'a';
	sq_to_algeb_puff[1]=(BOARD_HEIGHT-1-r)+'1';
	sq_to_algeb_puff[2]=0;

	return sq_to_algeb_puff;
}

char move_algeb_puff[6];
char* Move::algeb()
{
	strcpy_s(move_algeb_puff,6,square_to_algeb(from_sq));
	strcat_s(move_algeb_puff,6,square_to_algeb(to_sq));

	if(info & PROMOTION)
	{
		move_algeb_puff[4]=piece_letters[INFO_OF(info)];
		move_algeb_puff[5]=0;
	}

	return move_algeb_puff;
}

void Move::print()
{
	cout << algeb() << " depth " << (int)depth << " value " << value << endl;
}

void Position::list_legal_moves()
{
	
	init_move_generator();

	int legal_move_count=0;
	while(next_legal_move())
	{
		legal_move_count++;
		cout << try_move.algeb() << " ";
	}

	cout << "( number of legal moves = " << legal_move_count << " ) " << endl;

}

void Position::next_sq()
{
	while(current_sq<BOARD_SIZE)
	{

		from_piece=board[current_sq];
		if((from_piece)&&(TURN_OF(from_piece)==turn))
		{
			current_ptr=move_table_ptr[current_sq][from_piece];
			return;
		}

		current_sq++;
	}
}

void Position::init_move_generator()
{
	current_sq=0;

	next_sq();

	hash_done=false;
	hash_move.from_sq=SQUARE_NONE;
}

bool Position::next_pseudo_legal_move()
{

	while(current_sq<BOARD_SIZE)
	{

		while(current_ptr->info!=END_PIECE)
		{

			////////////////////////////////////////////////////////////
			// prepare move

			try_move=*current_ptr;

			to_sq=try_move.to_sq;
			to_piece=board[to_sq];

			capture=false;
			if((to_piece)&&(TURN_OF(to_piece)!=turn))
			{
				try_move.info|=CAPTURE;
				capture=true;
			}


			////////////////////////////////////////////////////////////
			// process move

			if(try_move.info & SLIDING_MOVE)
			{
				if(to_piece)
				{

					current_ptr=current_ptr->next_vector;

					if(capture)
					{
						return pseudo_legal_move_available=true;
					}
					
				}
				else
				{

					current_ptr++;
					return pseudo_legal_move_available=true;

				}
			}
			else if(try_move.info & SINGLE_MOVE)
			{

				if(try_move.info & CASTLING)
				{
					current_ptr++;

					if(try_move.to_sq==CASTLING_EMPTY_SQUARE_W_K_2)
					{

						bool right=((castling_rights&CASTLING_RIGHT_K)!=0);
						bool empty1=(!board[CASTLING_EMPTY_SQUARE_W_K_1]);
						bool empty2=(!board[CASTLING_EMPTY_SQUARE_W_K_2]);
						bool check1=(!is_in_check_square(WHITE_KING_CASTLE_FROM_SQUARE,WHITE));
						bool check2=(!is_in_check_square(CASTLING_EMPTY_SQUARE_W_K_1,WHITE));

						if(right && empty1 && empty2 && check1 && check2)
						{
							return pseudo_legal_move_available=true;
						}
					}

					if(try_move.to_sq==CASTLING_EMPTY_SQUARE_W_Q_2)
					{

						bool right=((castling_rights&CASTLING_RIGHT_Q)!=0);
						bool empty1=(!board[CASTLING_EMPTY_SQUARE_W_Q_1]);
						bool empty2=(!board[CASTLING_EMPTY_SQUARE_W_Q_2]);
						bool empty3=(!board[CASTLING_EMPTY_SQUARE_W_Q_3]);
						bool check1=(!is_in_check_square(WHITE_KING_CASTLE_FROM_SQUARE,WHITE));
						bool check2=(!is_in_check_square(CASTLING_EMPTY_SQUARE_W_Q_1,WHITE));

						if(right && empty1 && empty2 && empty3 && check1 && check2)
						{
							return pseudo_legal_move_available=true;
						}
					}

					if(try_move.to_sq==CASTLING_EMPTY_SQUARE_B_K_2)
					{

						bool right=((castling_rights&CASTLING_RIGHT_k)!=0);
						bool empty1=(!board[CASTLING_EMPTY_SQUARE_B_K_1]);
						bool empty2=(!board[CASTLING_EMPTY_SQUARE_B_K_2]);
						bool check1=(!is_in_check_square(BLACK_KING_CASTLE_FROM_SQUARE,BLACK));
						bool check2=(!is_in_check_square(CASTLING_EMPTY_SQUARE_B_K_1,BLACK));

						if(right && empty1 && empty2 && check1 && check2)
						{
							return pseudo_legal_move_available=true;
						}
					}

					if(try_move.to_sq==CASTLING_EMPTY_SQUARE_B_Q_2)
					{

						bool right=((castling_rights&CASTLING_RIGHT_q)!=0);
						bool empty1=(!board[CASTLING_EMPTY_SQUARE_B_Q_1]);
						bool empty2=(!board[CASTLING_EMPTY_SQUARE_B_Q_2]);
						bool empty3=(!board[CASTLING_EMPTY_SQUARE_B_Q_3]);
						bool check1=(!is_in_check_square(BLACK_KING_CASTLE_FROM_SQUARE,BLACK));
						bool check2=(!is_in_check_square(CASTLING_EMPTY_SQUARE_B_Q_1,BLACK));

						if(right && empty1 && empty2 && empty3 && check1 && check2)
						{
							return pseudo_legal_move_available=true;
						}
					}
					
				}
				else if(to_piece)
				{

					current_ptr++;

					if(capture)
					{
						return pseudo_legal_move_available=true;
					}
					
				}
				else
				{

					current_ptr++;
					return pseudo_legal_move_available=true;

				}

			}
			else
			{

				// pawn moves

				if(current_ptr->info & CAPTURE)
				{

					// capture
					pawn_push_possible=false;
					prom_piece=KNIGHT;

					current_ptr++;

					if(
						((to_piece)&&(TURN_OF(to_piece)!=turn))
						||
						(to_sq==ep_square)
					)
					{
						if(to_sq==ep_square)
						{
							try_move.info|=EP_CAPTURE;
						}
						return pseudo_legal_move_available=true;
					}
					
				}
				else
				{
					// push

					if(try_move.info & PAWN_PUSH_BY_TWO)
					{
						// push by two

						current_ptr++;

						if((!to_piece)&&pawn_push_possible)
						{
							return pseudo_legal_move_available=true;
						}
					}
					else
					{
						// push by one

						current_ptr++;

						if(!to_piece)
						{
							pawn_push_possible=true;

							if(try_move.info & PROMOTION)
							{
								try_move.info|=prom_piece;
								prom_piece++;
								if(prom_piece==VOID_PIECE){prom_piece=ROOK;}
								if(prom_piece<=QUEEN)
								{
									current_ptr--;
								}
							}

							return pseudo_legal_move_available=true;
						}
						
					}

				}

			}

			////////////////////////////////////////////////////////////

		}

		current_sq++;

		next_sq();

	}

	return pseudo_legal_move_available=false;

}

void Position::make_move(Move m)
{

	ep_square=SQUARE_NONE;

	Piece from_piece=board[m.from_sq];
	board[m.from_sq]=NO_PIECE;

	if(m.info & CAPTURE)
	{
		board[m.to_sq]=NO_PIECE;
		Move* start_ptr=move_table_ptr[m.to_sq][KING];

		while(! (start_ptr->info & END_PIECE) )
		{
			Square to_sq=start_ptr->to_sq;

			if(PIECE_OF(board[to_sq])!=PAWN)
			{
				board[to_sq]=NO_PIECE;
			}

			start_ptr++;
		}

		if(m.info & EP_CAPTURE)
		{
			board[INFO_OF(m.info)]=NO_PIECE;
		}

	}
	else
	{
		board[m.to_sq]=from_piece;
				
		if(from_piece==(WHITE_PIECE|KING))
		{
			king_pos[WHITE]=m.to_sq;

			if(m.info & CASTLING)
			{
				if(m.to_sq==CASTLING_EMPTY_SQUARE_W_K_2)
				{
					board[CASTLING_RIGHT_K_DISABLE_SQUARE]=NO_PIECE;
					board[CASTLING_EMPTY_SQUARE_W_K_1]=WHITE_PIECE|ROOK;
				}
				
				if(m.to_sq==CASTLING_EMPTY_SQUARE_W_Q_2)
				{
					board[CASTLING_RIGHT_Q_DISABLE_SQUARE]=NO_PIECE;
					board[CASTLING_EMPTY_SQUARE_W_Q_1]=WHITE_PIECE|ROOK;
				}
			}
		}
		else if(from_piece==(BLACK_PIECE|KING))
		{
			king_pos[BLACK]=m.to_sq;

			if(m.info & CASTLING)
			{
				if(m.to_sq==CASTLING_EMPTY_SQUARE_B_K_2)
				{
					board[CASTLING_RIGHT_k_DISABLE_SQUARE]=NO_PIECE;
					board[CASTLING_EMPTY_SQUARE_B_K_1]=BLACK_PIECE|ROOK;
				}
				
				if(m.to_sq==CASTLING_EMPTY_SQUARE_B_Q_2)
				{
					board[CASTLING_RIGHT_q_DISABLE_SQUARE]=NO_PIECE;
					board[CASTLING_EMPTY_SQUARE_B_Q_1]=BLACK_PIECE|ROOK;
				}
			}
		}

		if(m.info & PAWN_PUSH_BY_TWO)
		{
			ep_square=INFO_OF(m.info);
		}

		if(m.info & PROMOTION)
		{
			board[m.to_sq]=INFO_OF(m.info)|COLOR_OF(from_piece);
		}

	}

	if(!board[CASTLING_RIGHT_K_DISABLE_SQUARE]){castling_rights&=~CASTLING_RIGHT_K;}
	if(!board[CASTLING_RIGHT_Q_DISABLE_SQUARE]){castling_rights&=~CASTLING_RIGHT_Q;}
	if(!board[CASTLING_RIGHT_k_DISABLE_SQUARE]){castling_rights&=~CASTLING_RIGHT_k;}
	if(!board[CASTLING_RIGHT_q_DISABLE_SQUARE]){castling_rights&=~CASTLING_RIGHT_q;}
	if(king_pos[WHITE]!=WHITE_KING_CASTLE_FROM_SQUARE){castling_rights&=~(CASTLING_RIGHT_K|CASTLING_RIGHT_Q);}
	if(king_pos[BLACK]!=BLACK_KING_CASTLE_FROM_SQUARE){castling_rights&=~(CASTLING_RIGHT_k|CASTLING_RIGHT_q);}

	turn=(turn==WHITE?BLACK:WHITE);

	init_move_generator();
}

bool Position::is_algeb_move_legal(char* algeb)
{
	Square from_sq=algeb_to_square(algeb);
	Square to_sq=algeb_to_square(algeb+2);

	Move test;

	test.from_sq=from_sq;
	test.to_sq=to_sq;
	test.info=NO_INFO;

	if(algeb[4]!=0)
	{
		test.info|=PROMOTION;
		switch(algeb[4])
		{
			case 'n': test.info|=KNIGHT;break;
			case 'b': test.info|=BISHOP;break;
			case 'r': test.info|=ROOK;break;
			case 'q': test.info|=QUEEN;break;
		}
	}

	return is_move_legal(test);
}

int Position::calc_heuristic_value()
{
	calc_material_balance();
	calc_mobility_balance();
	calc_attacker_balance();

	return heuristic_value=
		material_balance
		+
		mobility_balance*MOBILITY_BONUS
		+
		attacker_balance*ATTACKER_BONUS
		+
		randh()
	;
}

time_t ltime0;
time_t ltime;
int nodes;
int depth_reached;

#ifdef QUIESCENCE_SEARCH
int Position::search_recursive(Depth depth,Depth max_depth,int alpha,int beta,bool maximizing,string line)
{
	nodes++;

	if(depth>depth_reached)
	{
		depth_reached=depth;
		if(verbose)
		{
			//cout << "depth reached " << depth_reached << " at " << line << endl;
		}
	}

	int value=maximizing?-INFINITE_SCORE:INFINITE_SCORE;

	init_move_generator();

	bool legal_move_found=false;
	bool search_move_found=false;

	if(next_legal_move())
	{

		legal_move_found=true;

		do
		{

			bool is_capture=((try_move.info&CAPTURE)||(try_move.info&EP_CAPTURE));
			bool is_legitimate_search_move=(is_capture||(depth<=max_depth));

			if(is_legitimate_search_move)
			{

				search_move_found=true;

				Position dummy=*this;

				#ifdef CALC_LINE
				string old_line=line;
				line+=try_move.algeb();
				line+=" ";
				#endif

				dummy.make_move(try_move);

				if(depth==0)
				{
					if(verbose&&(!quit_search))
					{
						cout << try_move.algeb() << " : ";
					}
				}

				int eval=dummy.search_recursive(depth+1,max_depth,alpha,beta,!maximizing,line);

				if(depth==0)
				{
					time( &ltime );int elapsed=(int)ltime-(int)ltime0;

					int nps=elapsed==0?0:(int)(nodes/elapsed/1e3);

					if(verbose&&(!quit_search))
					{
						cout 
							<< "eval " << eval 
							<< " nodes " << nodes 
							<< " nps " << nps 
							<< " kN/s reached " << depth_reached 
							<< " rand " << randh()
							<< endl;
					}
				}

				#ifdef CALC_LINE
				line=old_line;
				#endif

				if(maximizing)
				{
					if(eval>value)
					{

						value=eval;

						try_move.depth=depth;
						try_move.value=value;
						store_move(try_move);

					}
			
					if(depth>=0)
					{
						if(value>alpha){alpha=value;}
					}

				}
				else
				{

					if(eval<value)
					{

						value=eval;

						try_move.depth=depth;
						try_move.value=value;
						store_move(try_move);
			
					}

					if(depth>=0)
					{
						if(value<beta){beta=value;}
					}

				}

				if(beta<alpha)
				{
					return value;
				}

				if(quit_search)
				{
					return value;
				}

			}

		}while(next_legal_move());

	}
	
	// end of recursive search

	if(legal_move_found)
	{

		if(search_move_found)
		{

			return value;

		}
		else
		{

			return calc_heuristic_value();

		}

	}
	else
	{

		if(is_in_check(turn))
		{
			if(turn==WHITE)
			{
				return -MATE_SCORE+depth;
			}
			else
			{
				return MATE_SCORE-depth;
			}
		}
		else
		{
			return DRAW_SCORE;
		}

	}

}
#else
int Position::search_recursive(Depth depth,Depth max_depth,int alpha,int beta,bool maximizing,string line)
{
	nodes++;

	init_move_generator();

	if(!next_legal_move())
	{
		if(is_in_check(turn))
		{
			if(turn==WHITE)
			{
				return -MATE_SCORE+depth;
			}
			else
			{
				return MATE_SCORE-depth;
			}
		}
		else
		{
			return DRAW_SCORE;
		}
	}

	if(depth>=max_depth)
	{
		return calc_heuristic_value();
	}

	init_move_generator();

	int value=maximizing?-INFINITE_SCORE:INFINITE_SCORE;

	calc_hash_key();

	while(next_legal_move())
	{

		Position dummy=*this;

		//string old_line=line;
		//line+=try_move.algeb();
		//line+=" ";

		dummy.make_move(try_move);

		if(depth==0)
		{
			#ifndef XBOARD_COMPATIBLE
			if(verbose&&(!quit_search))
			{
				cout << try_move.algeb() << " : ";
			}
			#endif
		}
		

		int eval=dummy.search_recursive(depth+1,max_depth,alpha,beta,!maximizing,line);

		if(depth==0)
		{
			time( &ltime );int elapsed=(int)ltime-(int)ltime0;

			int nps=elapsed==0?0:(int)(nodes/elapsed/1e3);

			#ifndef XBOARD_COMPATIBLE
			if(verbose&&(!quit_search))
			{
				cout << "eval " << eval << " nodes " << nodes << " nps " << nps << " kNodes / sec " << endl;
			}
			#endif
		}

		//line=old_line;

		if(maximizing)
		{
			if(eval>value)
			{

				value=eval;

				try_move.depth=depth;
				try_move.value=value;
				store_move(try_move);

			}

			
			if(depth>=0)
			{
				if(value>alpha){alpha=value;}
			}
		}
		else
		{
			if(eval<value)
			{

				value=eval;

				try_move.depth=depth;
				try_move.value=value;
				store_move(try_move);
			
			}

			if(depth>=0)
			{
				if(value<beta){beta=value;}
			}
		}

		if(beta<alpha)
		{
			return value;
		}

		if(quit_search)
		{
			return value;
		}

	}

	return value;
}
#endif

bool force_non_verbose=false;
int Position::search(Depth max_depth)
{

	search_quitted=false;

	nodes=0;
	depth_reached=0;

	time( &ltime0 );

	Move test;

	int value;
	for(int current_max_depth=1;current_max_depth<=max_depth;current_max_depth++)
	{
		verbose=current_max_depth>=max_depth;

		#ifdef XBOARD_COMPATIBLE
		verbose=false;
		#endif

		if(force_non_verbose){verbose=false;}

		value=search_recursive(0,current_max_depth,-INFINITE_SCORE,INFINITE_SCORE,turn==WHITE,"");
		if(!quit_search)
		{
			//cout << "depth " << current_max_depth << " --> best move ";

			calc_hash_key();

			test=hash_table[hash_key];

			//test.print();

			time( &ltime );int elapsed=(int)ltime-(int)ltime0;

			int nps=elapsed==0?0:(int)(nodes/elapsed/1e3);

			if(!force_non_verbose)
			{
				//cout << "max depth " << (int)max_depth << endl;
				cout << current_max_depth << " " << value << " " << elapsed*100 << " " << nodes << " " << test.algeb() << endl;
			}

		}
	}

	if(do_make_move)
	{
		cout << "move " << test.algeb() << endl;
		make_move(test);
	}

	if(!disable_search_quitted)
	{
		// in deep search reporting search quitted here is wrong, because the deep search loop has not ended yet
		search_quitted=true;
	}

	return value;
}

int Position::calc_material_balance()
{

	material_balance=0;

	for(Square sq=0;sq<BOARD_SIZE;sq++)
	{
		material_balance+=material_values[board[sq]];
	}

	return material_balance;
}

int Position::count_pseudo_legal_moves()
{
	init_move_generator();
	no_pseudo_legal_moves=0;
	while(next_pseudo_legal_move())
	{
		no_pseudo_legal_moves++;
	}
	return no_pseudo_legal_moves;
}

int Position::calc_mobility_balance()
{

	if(MOBILITY_BONUS==0)
	{
		return 0;
	}

	Position dummy=*this;

	dummy.turn=TURN_WHITE;
	mobility_balance=dummy.count_pseudo_legal_moves();
	dummy.turn=TURN_BLACK;
	mobility_balance-=dummy.count_pseudo_legal_moves();

	return mobility_balance;
}

int Position::attackers_on_king(Turn attacked_side)
{

	Move* ptr=move_table_ptr[king_pos[attacked_side]][KING];

	int attackers=0;

	while(ptr->info!=END_PIECE)
	{
		Square attacked_square=ptr->to_sq;

		for(Piece test_piece=KNIGHT;test_piece<=QUEEN;test_piece++)
		{

			Move* ptr2=move_table_ptr[attacked_square][test_piece];

			Square test_sq;
			Piece curr_piece;

			while(ptr2->info!=END_PIECE)
			{

				test_sq=ptr2->to_sq;

				curr_piece=board[test_sq];

				if(!curr_piece)
				{
					ptr2++;
				}
				else
				{
					if((PIECE_OF(curr_piece)==test_piece)&&(SIDE_OF(curr_piece)!=attacked_side))
					{
						attackers++;
					}

					if(ptr2->info & SLIDING_MOVE)
					{
						ptr2=ptr2->next_vector;
					}
					else
					{
						ptr2++;
					}
				}
			}
		}

		ptr++;
	}

	return attackers;
}

int Position::calc_attacker_balance()
{

	if(ATTACKER_BONUS==0)
	{
		return 0;
	}

	attacker_balance=-attackers_on_king(WHITE)+attackers_on_king(BLACK);

	return attacker_balance;
}

bool Position::is_exploded(Turn side)
{
	return side==WHITE?(board[king_pos[WHITE]]!=(WHITE_PIECE|KING)):(board[king_pos[BLACK]]!=(BLACK_PIECE|KING));
}

bool Position::is_in_check(Turn side)
{

	if(is_exploded(side)){return true;}

	if(adjacent_kings()){return false;}

	Square side_king_pos=king_pos[side];
		
	for(Piece p=KNIGHT;p<=QUEEN;p++)
	{
		
		Move* start_ptr=move_table_ptr[side_king_pos][p];

		Piece test_piece=side==WHITE?BLACK_PIECE|p:WHITE_PIECE|p;
		
		examine_check:

		Move test_move;

		if(start_ptr->info==END_PIECE){goto piece_finished;}

		test_move=*start_ptr;

		Piece query_piece=board[test_move.to_sq];

		if(test_piece==query_piece)
		{
			if(!((p==PAWN)&&(!(start_ptr->info & CAPTURE))))
			{
				return true;
			}
		}

		if(!query_piece)
		{

			start_ptr++;

			goto examine_check;

		}
		else
		{

			if(test_move.info & SLIDING_MOVE)
			{
				start_ptr=test_move.next_vector;
			}
			else
			{
				start_ptr++;
			}

			goto examine_check;
			
		}

		piece_finished:

		int piece_done=1;

	}

	return false;
}

bool Position::adjacent_kings()
{

	if(is_exploded(WHITE)||is_exploded(BLACK)){return false;}

	Move* ptr=move_table_ptr[king_pos[WHITE]][KING];

	// if the white king can capture the black king the kings are adjacent
	while(!(ptr->info==END_PIECE))
	{
		if(ptr->to_sq==king_pos[BLACK]){return true;}
		ptr++;
	}

	return false;

}

bool Position::next_legal_move()
{

	if(!hash_done)
	{

		hash_done=true;

		calc_hash_key();

		Move test=hash_table[hash_key];
		
		if(is_move_legal(test))
		{
			try_move=test;
			hash_move=test;
			return true;
		}

	}

	while(next_pseudo_legal_move())
	{
		if(!try_move.equal_to(hash_move))
		{
			Position dummy=*this;

			dummy.make_move(try_move);

			if(
				(!dummy.is_exploded(turn))
				&&
				(
					(!dummy.is_in_check(turn))
					||
					(dummy.is_exploded(dummy.turn))
				)
			)
			{
				return legal_move_available=true;
			}
		}
	}

	return legal_move_available=false;
}

bool Move::equal_to(Move m)
{
	return(
		(from_sq==m.from_sq)
		&&
		(to_sq==m.to_sq)
		&&
		(info==m.info)
		);
}

bool Move::mainly_equal_to(Move m)
{
	return(
		(from_sq==m.from_sq)
		&&
		(to_sq==m.to_sq)
		);
}

bool Position::is_move_legal(Move m)
{
	Position dummy=*this;

	dummy.hash_done=true;
	dummy.hash_move.from_sq=SQUARE_NONE;

	dummy.current_sq=m.from_sq;

	Piece from_piece=dummy.board[dummy.current_sq];

	if((!from_piece)||(TURN_OF(from_piece)!=turn))
	{
		return false;
	}

	dummy.current_ptr=move_table_ptr[dummy.current_sq][from_piece];

	while((dummy.current_sq==m.from_sq)&&(dummy.next_legal_move()))
	{
		if(dummy.try_move.mainly_equal_to(m))
		{
			try_move=dummy.try_move;

			if(m.info & PROMOTION)
			{
				try_move.info=m.info;
			}

			return true;
		}
	}

	return false;
}

void Position::store_move(Move m)
{
	Move test=hash_table[hash_key];

	if((m.depth<=test.depth)||(test.from_sq==SQUARE_NONE))
	{
		hash_table[hash_key]=m;
	}
}

bool Position::is_in_check_square(Square sq,Turn side)
{
	Position dummy=*this;

	dummy.king_pos[side]=sq;
	dummy.board[sq]=(side==WHITE?WHITE_PIECE|KING:BLACK_PIECE|KING);

	return dummy.is_in_check(side);
}

void Position::save()
{
	ofstream o("pos.txt",ios::binary);
	o.write((char*)this,sizeof(Position));
	o.close();
}

void Position::load()
{
	ifstream i("pos.txt",ios::binary);
	i.read((char*)this,sizeof(Position));
	i.close();
}