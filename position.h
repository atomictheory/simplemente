
#ifndef POSITION_H
#define POSITION_H
#endif

#ifndef XBOARD_H
#include "xboard.h"
#endif

#include <string>

using namespace std;

extern bool quit_search;
extern bool search_quitted;
extern bool disable_search_quitted;
extern bool do_make_move;

//#define MY_DEBUG_POSITION_PRINT
//#define MY_DEBUG_MOVE_GEN

#define BOARD_WIDTH_SHIFT (3)
#define BOARD_WIDTH (1 << BOARD_WIDTH_SHIFT)
#define BOARD_WIDTH_I (BOARD_WIDTH-1)

#define BOARD_HEIGHT_SHIFT (3)
#define BOARD_HEIGHT (1 << BOARD_HEIGHT_SHIFT)
#define BOARD_HEIGHT_I (BOARD_HEIGHT-1)

#define BOARD_SIZE (BOARD_WIDTH*BOARD_HEIGHT)

#define WHITE (1)
#define TURN_WHITE WHITE
#define WHITE_PIECE (1 << 3)

#define BLACK (0)
#define TURN_BLACK BLACK
#define BLACK_PIECE (0)

#define COLOR_OF(P) (P & WHITE_PIECE)
#define TURN_OF(P) (COLOR_OF(P) >> 3)
#define SIDE_OF(P) TURN_OF(P)

#define CASTLING_RIGHT_NONE (0)
#define SQUARE_NONE (1 << 7)

#define SLIDING (1 << 2)
#define DIAGONAL (1 << 1)
#define STRAIGHT (1 << 0)

// no piece
#define NO_PIECE (0) // 0

// non sliding pieces
#define KING (1 << 0) // 1
#define KNIGHT (1 << 1) // 2
#define VOID_PIECE (3) // 3

// pawn is defined as a sliding piece which cannot move in any direction
#define PAWN (SLIDING) // 4

// sliding pieces
#define ROOK (SLIDING|STRAIGHT) // 5
#define BISHOP (SLIDING|DIAGONAL) // 6
#define QUEEN (SLIDING|STRAIGHT|DIAGONAL) // 7

#define CASTLING_RIGHT_K (1 << 3)
#define CASTLING_RIGHT_K_DISABLE_SQUARE (63)
#define CASTLING_RIGHT_Q (1 << 2)
#define CASTLING_RIGHT_Q_DISABLE_SQUARE (56)
#define CASTLING_RIGHT_W CASTLING_RIGHT_K|CASTLING_RIGHT_Q
#define CASTLING_RIGHT_k (1 << 1)
#define CASTLING_RIGHT_k_DISABLE_SQUARE (7)
#define CASTLING_RIGHT_q (1)
#define CASTLING_RIGHT_q_DISABLE_SQUARE (0)
#define CASTLING_RIGHT_b CASTLING_RIGHT_k|CASTLING_RIGHT_q
//#define CASTLING_RIGHT_K_OF(c) (c==WHITE?CASTLING_RIGHT_K:CASTLING_RIGHT_k)
//#define CASTLING_RIGHT_Q_OF(c) (c==WHITE?CASTLING_RIGHT_Q:CASTLING_RIGHT_q)
#define CASTLING_EMPTY_SQUARE_W_K_1 (61)
#define CASTLING_EMPTY_SQUARE_W_K_2 (62)
#define CASTLING_EMPTY_SQUARE_W_Q_1 (59)
#define CASTLING_EMPTY_SQUARE_W_Q_2 (58)
#define CASTLING_EMPTY_SQUARE_W_Q_3 (57)
#define CASTLING_EMPTY_SQUARE_B_K_1 (5)
#define CASTLING_EMPTY_SQUARE_B_K_2 (6)
#define CASTLING_EMPTY_SQUARE_B_Q_1 (3)
#define CASTLING_EMPTY_SQUARE_B_Q_2 (2)
#define CASTLING_EMPTY_SQUARE_B_Q_3 (1)
#define CASTLING_RIGHT_NONE (0)

#define WHITE_KING_CASTLE_FROM_SQUARE (60)
#define BLACK_KING_CASTLE_FROM_SQUARE (4)

#define FILE_MASK ((1 << BOARD_WIDTH_SHIFT)-1)
#define RANK_MASK (((1 << BOARD_HEIGHT_SHIFT)-1) << BOARD_WIDTH_SHIFT)

#define FILE_OF(SQ) (SQ & FILE_MASK)
#define RANK_OF(SQ) ((SQ & RANK_MASK) >> BOARD_WIDTH_SHIFT)

#define VALID_FILE(F) ((F>=0)&&(F<BOARD_WIDTH))
#define VALID_RANK(R) ((R>=0)&&(R<BOARD_HEIGHT))

#define FILE_OF(SQ) (SQ & FILE_MASK)
#define RANK_OF(SQ) ((SQ & RANK_MASK) >> BOARD_WIDTH_SHIFT)

#define SQUARE_FROM_FR(F,R) (F + (R << BOARD_WIDTH_SHIFT))

#define PIECE_MASK (7)
#define PIECE_OF(P) (P & PIECE_MASK)

const char piece_letters[]=" kn prbq KN PRBQ";

typedef unsigned char Square;
typedef unsigned char File;
typedef unsigned char Rank;
typedef unsigned char Piece;
typedef unsigned char Color;
typedef unsigned char Turn;
typedef unsigned char CastlingRight;

typedef unsigned char Depth;
typedef unsigned char MoveCount;

#define HASH_TABLE_SHIFT (21)
#define HASH_TABLE_SIZE (1 << HASH_TABLE_SHIFT)

const unsigned int HASH_TABLE_MASK=(HASH_TABLE_SIZE-1);

struct Move;
extern Move hash_table[HASH_TABLE_SIZE];

#define POSITION_TRUNK_SIZE (BOARD_SIZE+3)

typedef char PositionTrunk[POSITION_TRUNK_SIZE];

typedef unsigned int HashKey;
typedef unsigned short int MoveInfo;

// max number of white plus black pieces
#define MAX_PIECE (WHITE_PIECE << 1)

extern int material_values[MAX_PIECE];

////////////////////////////////////////////////////////////////////
// heuristic tuning

/*#define ATTACKER_BONUS (45)
#define MOBILITY_BONUS (15)

#ifdef XBOARD_COMPATIBLE
#define RANDOM_BONUS (100)
#else
#define RANDOM_BONUS (15)
#endif

#define KNIGHT_VALUE (150)
#define BISHOP_VALUE (150)
#define ROOK_VALUE (300)
#define QUEEN_VALUE (600)
#define PAWN_VALUE (100)*/

// conventional set

#define ATTACKER_BONUS (15)
#define MOBILITY_BONUS (10)

#ifdef XBOARD_COMPATIBLE
#define RANDOM_BONUS (100)
#else
#define RANDOM_BONUS (5)
#endif

#define KNIGHT_VALUE (300)
#define BISHOP_VALUE (300)
#define ROOK_VALUE (500)
#define QUEEN_VALUE (900)
#define PAWN_VALUE (120)

////////////////////////////////////////////////////////////////////

#define MATE_SCORE (10000)
#define DRAW_SCORE (0)
#define INFINITE_SCORE (MATE_SCORE+1)

// a queen can make at most 7 moves in 4 directions making up for 28 moves, so 30 must be enough
#define MAX_LEGAL_MOVES_PER_SQUARE (30)

#define END_PIECE (1 << 7) // 128
#define END_VECTOR (1 << 8) // 256
#define SINGLE_MOVE (1 << 9) // 512
#define SLIDING_MOVE (1 << 10) // 1024
#define CAPTURE (1 << 11) // 2048
#define EP_CAPTURE (1 << 12) // 4096
#define PAWN_PUSH_BY_TWO (1 << 13) // 8192
#define PROMOTION (1 << 14) // 16384
#define CASTLING (1 << 15) // 32768
#define PAWN_PUSH (0)
#define NO_INFO (0)

#define INFO_MASK ((1 << 6) - 1)
#define INFO_OF(I) (I & INFO_MASK)

#define MAX_DEPTH 20

struct Position;
struct Move
{
	Square from_sq;
	Square to_sq;
	MoveInfo info;

	Move* next_vector;

	Depth depth;
	int value;

	int original_search_value;
	int eval;

	char* algeb();
	void print();
	bool mainly_equal_to(Move);
	bool equal_to(Move);
};

extern Move* move_table_ptr[BOARD_SIZE][MAX_PIECE];

extern Move move_table[BOARD_SIZE*MAX_PIECE*MAX_LEGAL_MOVES_PER_SQUARE*2];

extern void init_move_table();

extern char* square_to_algeb(Square);

extern bool force_non_verbose;

struct Position
{
	// position trunk - to be used for generating a hash key
	Piece board[BOARD_SIZE];
	Turn turn;
	Square ep_square;
	CastlingRight castling_rights;
	// end position trunk

	int halfmove_clock;
	int fullmove_number;

	Square king_pos[2];

	void print();

	HashKey hash_key;
	HashKey calc_hash_key();

	Square algeb_to_square(char*);

	////////////////////////////////////
	// move generation
	bool hash_done;
	Move hash_move;
	Square current_sq;
	Move* current_ptr;
	Piece from_piece;
	Piece to_piece;
	Square to_sq;
	Move try_move;
	void next_sq();
	void init_move_generator();
	bool pseudo_legal_move_available;
	bool legal_move_available;
	bool capture;
	bool pawn_push_possible;
	Piece prom_piece;
	Square pawn_push_sq;
	bool next_pseudo_legal_move();
	bool next_legal_move();
	////////////////////////////////////

	void make_move(Move);

	bool is_move_legal(Move);
	bool is_algeb_move_legal(char*);
	void list_legal_moves();

	void reset();
	void set_from_fen(char*);

	bool process_normal_move();

	bool is_exploded(Turn);
	bool is_in_check(Turn);
	bool is_in_check_square(Square,Turn);
	bool adjacent_kings();

	////////////////////////////////////
	// search
	bool verbose;
	void store_move(Move);
	int attackers_on_king(Turn);
	int attacker_balance;
	int calc_attacker_balance();
	int no_pseudo_legal_moves;
	int count_pseudo_legal_moves();
	int mobility_balance;
	int calc_mobility_balance();
	int material_balance;
	int calc_material_balance();
	int search(Depth);
	int search_recursive(Depth,Depth,int,int,bool,string);
	int heuristic_value;
	int calc_heuristic_value();
	////////////////////////////////////

	void save();
	void load();

};

