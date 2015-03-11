#include "diskhash.h"

#include <direct.h>

#include <iostream>
#include <fstream>

#include "xxhash.h"

using namespace std;

#ifdef ALLOW_DATA
void create_data_directory()
{

	_mkdir("Data");

}

void init_disk_hash()
{

	create_data_directory();

}

PositionHashKey calc_disk_hash_key(Position* p)
{
	PositionHashKey disk_hash_key=(PositionHashKey)XXH32((void*)p,sizeof(PositionTrunk),0);

	disk_hash_key&=DISK_POSITION_HASH_MASK;

	return disk_hash_key;
}

char disk_hash_file_name[500];
void save_position_to_disk_hash(Position* p)
{

	BookPositionTableEntry* entry=book_look_up_position(p,DONT_CREATE);

	if(entry==NULL)
	{
		return;
	}

	PositionHashKey key=calc_disk_hash_key(p);

	sprintf_s(disk_hash_file_name,"Data/%d.smp",key);

	//cout << "saving position to " << disk_hash_file_name << endl;

	ofstream o(disk_hash_file_name,ios::binary|ios::app);

	o.write((char*)entry,sizeof(BookPositionTableEntry));

	Move* moves_buffer=&book_move_eval_table[entry->moves_ptr];
	int move_buffer_size=entry->no_moves*sizeof(Move);
	
	//cout << "size of Entry " << sizeof(BookPositionTableEntry) << endl;
	//cout << "size of Move " << sizeof(Move) << endl;
	//cout << "size of Move Buffer " << move_buffer_size << endl;

	o.write((char*)moves_buffer,move_buffer_size);

	o.close();

}

BookPositionTableEntry* book_look_up_position_callback(Position* p)
{

	PositionHashKey key=calc_disk_hash_key(p);

	sprintf_s(disk_hash_file_name,"Data/%d.smp",key);

	//cout << "looking up position at " << disk_hash_file_name << endl;

	ifstream i(disk_hash_file_name,ios::binary);
	if(i.is_open())
	{
		
		//cout << "found" << endl;

		BookPositionTableEntry entry_buffer;
		Move move_buffer[MAX_NUMBER_OF_LEGAL_MOVES_PER_POSITION];

		do
		{
			
			// read in entry
			i.read((char*)&entry_buffer,sizeof(BookPositionTableEntry));

			if(!i.eof())
			{

				int move_block_size=(sizeof(Move)*entry_buffer.no_moves);

				//cout << "contains " << (int)entry_buffer.no_moves << " moves, move block size " << move_block_size << endl;

				// read in moves
				i.read((char*)&move_buffer,move_block_size);

				if(0 == memcmp(entry_buffer.id,(char*)p,sizeof(PositionTrunk)))
				{

					//cout << "matches searched position" << endl;

					// we can only get here if position was not in memory, so we have to create it
					BookPositionTableEntry* entry=book_look_up_position_in_memory(p,DO_CREATE);

					if(entry!=NULL)
					{

						// there is room in memory to read in position from disk

						// first save pointers because those on disk are corrupt
						// other fields must be correct
						int next_old=entry->next;
						int moves_ptr_old=entry->moves_ptr;

						// now copy in entry from disk
						memcpy(entry,&entry_buffer,sizeof(BookPositionTableEntry));

						// correct pointers
						entry->next=next_old;
						entry->moves_ptr=moves_ptr_old;

						// now copy in moves
						Move* start_of_moves=&book_move_eval_table[moves_ptr_old];

						memcpy(start_of_moves,&move_buffer,move_block_size);

						// entry created
						i.close();
						return entry;

					}
					else
					{
						// out of memory
						return NULL;
					}

				}

			}

		}while(!i.eof());

		i.close();

		return NULL;

	}
	else
	{

		return NULL;

	}

}

void search_move_values_callback(Position* p)
{

	save_position_to_disk_hash(p);

}
#endif

void disk_size_info()
{

	cout << "disk hash size " << DISK_POSITION_HASH_SIZE << endl;
	cout << endl;

}