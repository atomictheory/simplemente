
#ifndef DEEP_H
#define DEEP_H
#endif

#ifndef POSITION_H
#include "position.h"
#endif

#ifndef MEMORY_H
#include "memory.h"
#endif

#ifdef ALLOW_DATA
extern void search_move_values_callback(Position*);
#endif

extern void minimax_out_and_sort(Position*);

#define MINIMAX_DEPTH (20)
#define CALC_PV_DEPTH (10)
#define MAX_LISTED_MOVES (10)

#define MINIMAX_AFTER 20

extern int minimax_counter;

// percentage chance with which the best move is played
// the higher the chance the deeper the search can go
#define SEARCH_DEEPNESS (20)
// increase this probability with depth
#define DEPTH_BONUS (20)
// cutoff
#define CUTOFF (MATE_SCORE - 100)

#define MIN(A,B) (A<B?A:B)

#define ESTIMATED_NUMBER_OF_LEGAL_MOVES_PER_POSITION (35)
#define MAX_NUMBER_OF_LEGAL_MOVES_PER_POSITION (350)

#define BOOK_HASH_SHIFT (17)

#define BOOK_POSITION_HASH_SIZE (1 << BOOK_HASH_SHIFT)
#define BOOK_POSITION_HASH_MASK (BOOK_POSITION_HASH_SIZE - 1)
#define BOOK_POSITION_HASH_LAST_INDEX BOOK_POSITION_HASH_MASK

#define BOOK_POSITION_TABLE_SIZE (BOOK_POSITION_HASH_SIZE)
#define BOOK_POSITION_TABLE_LAST_INDEX (BOOK_POSITION_TABLE_SIZE - 1)

#define BOOK_MOVE_EVAL_TABLE_SIZE (BOOK_POSITION_TABLE_SIZE * ESTIMATED_NUMBER_OF_LEGAL_MOVES_PER_POSITION)
#define BOOK_MOVE_EVAL_TABLE_LAST_INDEX (BOOK_MOVE_EVAL_TABLE_SIZE - 1)

struct BookPositionTableEntry
{
	PositionTrunk id;

	int next;

	MoveCount no_moves;

	bool search_done;

	int moves_ptr;
};

extern BookPositionTableEntry* book_look_up_position_callback(Position*);

// hash key points here
extern int book_hash_table[BOOK_POSITION_HASH_SIZE];

#define SIZEOF_BOOK_HASH_TABLE (sizeof(book_hash_table))

// hash table entry points here
extern int book_position_table_alloc_ptr;
extern BookPositionTableEntry book_position_table[BOOK_POSITION_TABLE_SIZE];

// position move points here
extern int book_move_eval_table_alloc_ptr;
extern Move book_move_eval_table[BOOK_MOVE_EVAL_TABLE_SIZE];

typedef unsigned int PositionHashKey;

extern void init_book();

extern PositionHashKey calc_book_hash_key(Position*);

#define DONT_CREATE false
#define DO_CREATE true
extern BookPositionTableEntry* book_look_up_position_in_memory(Position*,bool);
extern BookPositionTableEntry* book_look_up_position(Position*,bool);

extern void list_move_values(Position* p);
extern void search_move_values(Position* p);

extern void book_size_info();

extern void save_book();
extern void load_book();

extern void sort_moves(Position*);

extern void minimax_out(Position*);

extern bool add_node(Position*);

extern char* calc_pv(Position*);

extern char* value_nice(int);