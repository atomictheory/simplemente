#include "deep.h"

#include "xxhash.h"
#include <memory.h>

#include <iostream>
#include <fstream>

#include <stdlib.h>

using namespace std;

// hash key points here
int book_hash_table[BOOK_POSITION_HASH_SIZE];

// hash table entry points here
int book_position_table_alloc_ptr;
BookPositionTableEntry book_position_table[BOOK_POSITION_HASH_SIZE];

// position move points here
int book_move_eval_table_alloc_ptr;
Move book_move_eval_table[BOOK_MOVE_EVAL_TABLE_SIZE];

void init_book()
{
	memset(&book_hash_table,0,SIZEOF_BOOK_HASH_TABLE);

	// 0 is reserved for NULL pointer
	book_position_table_alloc_ptr=1;

	book_move_eval_table_alloc_ptr=0;
}

PositionHashKey calc_book_hash_key(Position* p)
{
	PositionHashKey book_hash_key=(PositionHashKey)XXH32((void*)p,sizeof(PositionTrunk),0);

	book_hash_key&=BOOK_POSITION_HASH_MASK;

	return book_hash_key;
}

int alloc_new_position_table_entry(Position *p)
{

	// allocation fails if out of move eval memory ...

	if(book_move_eval_table_alloc_ptr>=(BOOK_MOVE_EVAL_TABLE_LAST_INDEX-MAX_NUMBER_OF_LEGAL_MOVES_PER_POSITION))
	{

		// out of move eval memory

		return 0;
	}

	// ... or position memory

	if(book_position_table_alloc_ptr>=BOOK_POSITION_HASH_LAST_INDEX)
	{

		// out of position memory

		return 0;

	}

	// everything is ok

	int allocated_ptr=book_position_table_alloc_ptr++;

	BookPositionTableEntry* new_position_table_entry=&book_position_table[allocated_ptr];

	memcpy((char*)new_position_table_entry->id,(char*)p,sizeof(PositionTrunk));

	new_position_table_entry->no_moves=0;
	new_position_table_entry->next=NULL;

	new_position_table_entry->search_done=false;

	new_position_table_entry->moves_ptr=book_move_eval_table_alloc_ptr;

	p->init_move_generator();
	while(p->next_legal_move())
	{

		// move has not been evaluated yet
		p->try_move.value=-INFINITE_SCORE;
		p->try_move.eval=-INFINITE_SCORE;

		// record move
		book_move_eval_table[book_move_eval_table_alloc_ptr++]=p->try_move;
		new_position_table_entry->no_moves++;

	}

	return allocated_ptr;

}

BookPositionTableEntry* book_look_up_position(Position* p,bool create)
{
	PositionHashKey position_hash_key=calc_book_hash_key(p);

	int entry_ptr=book_hash_table[position_hash_key];

	if(entry_ptr==NULL)
	{

		if(!create)
		{
			return NULL;
		}

		// hash key was not used before

		int allocated_ptr=alloc_new_position_table_entry(p);

		// check if allocation was successful

		if(allocated_ptr==NULL)
		{

			// out of memory

			return NULL;

		}

		book_hash_table[position_hash_key]=allocated_ptr;

		return &book_position_table[allocated_ptr];

	}
	else
	{

		// see if position is found in the chained list

		BookPositionTableEntry* entry=&book_position_table[entry_ptr];

		while((entry->next!=NULL)&&(0 != (memcmp(entry->id,(char*)p,sizeof(PositionTrunk)))))
		{

			entry=&book_position_table[entry->next];

		}

		if(0 != (memcmp(entry->id,(char*)p,sizeof(PositionTrunk))))
		{

			if(!create)
			{
				return NULL;
			}

			// not found, new position has to be allocated ...

			int allocated_ptr=alloc_new_position_table_entry(p);

			if(allocated_ptr==NULL)
			{

				// out of memory

				return NULL;

			}

			// ... and linked to the list ( out of memory is taken care of by the NULL pointer! )

			entry->next=allocated_ptr;

			return &book_position_table[allocated_ptr];

		}

		// position found

		return entry;

	}

}

void list_move_values(Position* p)
{

	BookPositionTableEntry* entry=book_look_up_position(p,DO_CREATE);

	if(entry==NULL)
	{
		cout << "position not found in book" << endl;

		return;
	}

	sort_moves(p);

	cout << "position found at " << (int)(entry-book_position_table) << endl;
	cout << endl;
	
	for(int i=0;i<MIN(entry->no_moves,10);i++)
	{

		Move m=book_move_eval_table[entry->moves_ptr+i];

		cout 
			<< ((i<9)?" ":"") << (i+1) << ". " 
			<< m.algeb() 
			<< " = " 
			<< value_nice(m.eval)
			<< " ( orig " 
			<< value_nice(m.value)
			<< " ) ";

		Position dummy=*p;

		dummy.make_move(m);

		cout << (book_look_up_position(&dummy,DONT_CREATE)==NULL?"":"-->") << endl;

	}

	cout << endl;
}

bool search_move_values_verbose=true;
void search_move_values(Position* p)
{

	BookPositionTableEntry* entry=book_look_up_position(p,DO_CREATE);

	if(entry==NULL)
	{
		cout << "out of memory" << endl;

		return;
	}

	force_non_verbose=true;
	for(int i=0;i<entry->no_moves;i++)
	{
		Position dummy=*p;

		if(search_move_values_verbose)
		{
			cout << book_move_eval_table[entry->moves_ptr+i].algeb();
		}

		dummy.make_move(book_move_eval_table[entry->moves_ptr+i]);

		
		int value=dummy.search(4);

		if(search_move_values_verbose)
		{
			cout << " = " << value << " ";
		}

		book_move_eval_table[entry->moves_ptr+i].value=value;
		// sort is based on eval, so eval has to be set to search value
		book_move_eval_table[entry->moves_ptr+i].eval=value;

	}
	force_non_verbose=false;

	cout << endl;

	entry->search_done=true;

}

void book_size_info()
{

	cout << "position hash size " << BOOK_POSITION_HASH_SIZE << endl;
	cout << "number of positions " << (book_position_table_alloc_ptr-1) << endl;
	cout << "move book size " << BOOK_MOVE_EVAL_TABLE_SIZE << endl;
	cout << "number of moves " << book_move_eval_table_alloc_ptr << endl;
	//cout << "position table entry size " << sizeof(BookPositionTableEntry) << endl;
	//cout << "position book size " << sizeof(book_position_table) << endl;
	//cout << "move size " << sizeof(Move) << endl;
	//cout << "move book size " << sizeof(book_move_eval_table) << endl;
	cout << "number of moves " << book_move_eval_table_alloc_ptr << endl;
	cout << endl;
}

void save_book()
{
	ofstream o("book.smp",ios::binary);
	o.write((char*)&book_position_table_alloc_ptr,sizeof(book_position_table_alloc_ptr));
	o.write((char*)&book_move_eval_table_alloc_ptr,sizeof(book_move_eval_table_alloc_ptr));
	o.write((char*)&book_hash_table,sizeof(book_hash_table));
	o.write((char*)&book_position_table,sizeof(book_position_table));
	o.write((char*)&book_move_eval_table,sizeof(book_move_eval_table));
	o.close();
}

void load_book()
{
	ifstream i("book.smp",ios::binary);
	i.read((char*)&book_position_table_alloc_ptr,sizeof(book_position_table_alloc_ptr));
	i.read((char*)&book_move_eval_table_alloc_ptr,sizeof(book_move_eval_table_alloc_ptr));
	i.read((char*)&book_hash_table,sizeof(book_hash_table));
	i.read((char*)&book_position_table,sizeof(book_position_table));
	i.read((char*)&book_move_eval_table,sizeof(book_move_eval_table));
	i.close();
}

int cmp_moves_turn_white(const void* a,const void* b)
{
	return( ((Move*)b)->eval - ((Move*)a)->eval );
}

int cmp_moves_turn_black(const void* a,const void* b)
{
	return( ((Move*)a)->eval - ((Move*)b)->eval );
}

void sort_moves(Position* p)
{

	BookPositionTableEntry* entry=book_look_up_position(p,DONT_CREATE);

	if(entry==NULL)
	{
		return;
	}

	Move* moves=&book_move_eval_table[entry->moves_ptr];

	qsort(moves,entry->no_moves,sizeof(Move),p->turn==WHITE?cmp_moves_turn_white:cmp_moves_turn_black);

}

int minimax_recursive(int depth,Position* p)
{

	BookPositionTableEntry* entry=book_look_up_position(p,DONT_CREATE);

	if(entry==NULL)
	{

		return -INFINITE_SCORE;

	}

	if(entry->no_moves==0)
	{

		if(p->is_in_check(p->turn))
		{

			return p->turn==WHITE?-MATE_SCORE+depth:MATE_SCORE-depth;

		}
		else
		{

			return DRAW_SCORE;

		}

	}

	if(!entry->search_done)
	{

		return -INFINITE_SCORE;

	}
	
	int value=p->turn==WHITE?-INFINITE_SCORE:INFINITE_SCORE;

	for(int i=0;i<entry->no_moves;i++)
	{

		Position dummy=*p;

		dummy.make_move(book_move_eval_table[entry->moves_ptr+i]);

		int eval=minimax_recursive(depth+1,&dummy);

		if(eval!=-INFINITE_SCORE)
		{

			book_move_eval_table[entry->moves_ptr+i].eval=eval;

		}
		else
		{
			eval=book_move_eval_table[entry->moves_ptr+i].value;
			book_move_eval_table[entry->moves_ptr+i].eval=eval;

		}

		if(p->turn==WHITE)
		{
			if(eval>value){value=eval;}
		}
		else
		{
			if(eval<value){value=eval;}
		}

	}

	return value;

}

void minimax_out(Position* p)
{

	minimax_recursive(0,p);

}

bool add_node_recursive(Position* p)
{

	BookPositionTableEntry* entry=book_look_up_position(p,DONT_CREATE);

	if(entry==NULL)
	{
		search_move_values(p);
		return true;
	}

	if(entry->no_moves==0)
	{
		return false;
	}

	if(!entry->search_done)
	{
		search_move_values(p);
		return true;
	}

	sort_moves(p);
	
	Move m=book_move_eval_table[entry->moves_ptr];

	if((entry->no_moves>1)&&((rand()%100)>SEARCH_DEEPNESS))
	{
		m=book_move_eval_table[entry->moves_ptr+1];

		if((entry->no_moves>2)&&((rand()%100)>SEARCH_DEEPNESS))
		{
			m=book_move_eval_table[entry->moves_ptr+2];
		}
	}

	cout <<  m.algeb() << " ";

	Position dummy=*p;

	dummy.make_move(m);

	return add_node_recursive(&dummy);

}

bool add_node(Position* p)
{

	minimax_out(p);

	cout << "positions " << book_position_table_alloc_ptr << endl << endl << "examining ";

	search_move_values_verbose=false;
	bool node_added=add_node_recursive(p);
	search_move_values_verbose=true;

	cout << endl;

	return node_added;

}

char pv_puff[500];
void calc_pv_recursive(Position* p)
{

	BookPositionTableEntry* entry=book_look_up_position(p,DONT_CREATE);

	if(entry==NULL)
	{

		return;

	}

	if(entry->no_moves==0)
	{

		return;

	}

	if(!entry->search_done)
	{

		return;

	}
	
	Position dummy=*p;

	Move m=book_move_eval_table[entry->moves_ptr];

	strcat_s(pv_puff,500,m.algeb());
	strcat_s(pv_puff,500," ");

	dummy.make_move(m);

	calc_pv_recursive(&dummy);

}

char* calc_pv(Position* p)
{

	pv_puff[0]=0;

	calc_pv_recursive(p);

	return pv_puff;

}

char value_nice_puff[50];
char value_nice_itoa_puff[50];
char* value_nice(int value)
{
	if(value==-INFINITE_SCORE)
	{
		strcpy_s(value_nice_puff,"?");
	}
	else
	{
		_itoa_s(value,value_nice_itoa_puff,50,10);
		strcpy_s(value_nice_puff,value_nice_itoa_puff);
	}
	return value_nice_puff;
}