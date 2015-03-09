#include "simplemente.h"
#include "xboard.h"

#include "position.h"

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <process.h>

#include "deep.h"

using namespace std;

Position p;

Position game[200];
int game_ptr=0;

void init_main()
{
	init_move_table();

	init_book();

	p.reset();

	reset_game();
}

void search_thread(void* param)
{
	p.search(*((int*)param));
}

int add_node_cnt=0;
void add_nodes_thread(void* param)
{
	search_quitted=false;
	while(!quit_search)
	{
		if(add_node(&p))
		{

			list_move_values(&p);

			cout << "pv " << calc_pv(&p) << endl << endl;

			add_node_cnt++;

			if((add_node_cnt%20)==0)
			{
				save_book();

				book_size_info();
			}

		}
		else
		{
			cout << "warning : node generation failed" << endl;
			Sleep(1000);

		}

	}
	search_quitted=true;
}

#ifndef XBOARD_COMPATIBLE
char buf[100];
void main(int argc,char** argv)
{

	init_main();

	search_quitted=true;

	do
	{

		if(search_quitted)
		{
		
			p.print();

			list_move_values(&p);

		}
		

		cin.getline(buf,100);
		
		if(0==strcmp(buf,"help"))
		{
			cout << "help: " << endl;
			cout << endl;

			cout << "command format: [one letter command][argument]" << endl;
			cout << endl;

			cout << "r : reset position" << endl;
			cout << "f : set position from fen on clipboard" << endl;
			cout << "l : list legal moves in position" << endl;
			cout << "m[algeb] : make move given in algebraic notation ( example 'me2e4' )" << endl;
			cout << "u : unmake move" << endl;
			cout << "p : save position to disk" << endl;
			cout << "o : load position from disk" << endl;
			cout << "s : save book to disk" << endl;
			cout << "h : load book from disk" << endl;
			cout << "y : load position and book from disk" << endl;
			cout << "v : search moves in position" << endl;
			cout << "g[depth] : search to depth with alphabeta, depth= 1 .. 9" << endl;
			cout << "a : start deep analysis" << endl;
			cout << "q : quit search or deep analysis" << endl;
			cout << "i : minimax out book to current position" << endl;
			cout << "help : display this help" << endl;
			cout << "ENTER : print position" << endl;

			cout << "x : exit" << endl;

			cout << endl;
			cout << "press ENTER to continue" << endl;

			cin.getline(buf,100);
		}


		if(buf[0]=='r')
		{
			p.print();
		}

		if(buf[0]=='p')
		{
			p.save();
		}

		if(buf[0]=='o')
		{
			p.load();
		}

		if(buf[0]=='y')
		{
			load_book();
			p.load();
		}

		if(buf[0]=='v')
		{
			search_move_values(&p);
		}

		if(buf[0]=='a')
		{
			quit_search=false;
			search_quitted=false;
			_beginthread(add_nodes_thread,0,NULL);
		}

		if(buf[0]=='i')
		{
			minimax_out(&p);
		}

		if(buf[0]=='r')
		{
			p.reset();
			p.print();
			game_ptr=0;
		}

		if(buf[0]=='s')
		{
			save_book();
			book_size_info();
			cout << "book saved" << endl;
		}

		if(buf[0]=='h')
		{
			load_book();
			book_size_info();
			cout << "book loaded" << endl;
		}

		if(buf[0]=='f')
		{
			OpenClipboard(NULL);
			char fen[200];
			strncpy_s(fen, (char*)GetClipboardData(CF_TEXT),200);
			CloseClipboard();

			p.set_from_fen(fen);
			p.print();
			game_ptr=0;
		}

		if(buf[0]=='m')
		{
			if(p.is_algeb_move_legal(buf+1))
			{
				game[game_ptr++]=p;
				p.make_move(p.try_move);
				p.print();
			}
			else
			{
				cout << "illegal move" << endl;
			}
		}

		if(buf[0]=='u')
		{
			if(game_ptr-->0)
			{
				p=game[game_ptr];
				p.print();
			}
		}

		if(buf[0]=='g')
		{
			if((buf[1]>='1')&&(buf[1]<='9'))
			{
				cout << endl;
				int search_depth=buf[1]-'0';
				search_quitted=false;
				quit_search=false;
				_beginthread(search_thread,0,&search_depth);
			}
		}

		if(buf[0]=='q')
		{
			quit_search_safe();
		}

		if(buf[0]=='l')
		{
			p.list_legal_moves();
		}

	}while(buf[0]!='x');
}
#endif

/////////////////////////////////////////////////////////////

void quit_search_safe()
{
	quit_search=true;
	while(!search_quitted)
	{
		Sleep(50);
	}
	quit_search=false;
}

void reset_game()
{
	quit_search_safe();

	p.reset();
	game_ptr=0;
	analyze_mode=false;
}

int search_depth;
void analyze_and_make_move(const char* usermove)
{

	quit_search_safe();

	do_make_move=true;

	search_depth=6;
	if(p.is_algeb_move_legal((char*)usermove))
	{
		game[game_ptr++]=p;
		p.make_move(p.try_move);
		_beginthread(search_thread,0,&search_depth);
	}

}

void force()
{
	quit_search_safe();
}

void undo_move()
{

	quit_search_safe();

	if(game_ptr-->0)
	{
		p=game[game_ptr];
	}
}

void just_make_a_move()
{

	quit_search_safe();

	do_make_move=true;

	search_depth=6;
	_beginthread(search_thread,0,&search_depth);

}

void analyze_pos()
{

	do_make_move=false;

	search_depth=10;
	_beginthread(search_thread,0,&search_depth);
}

void analyze_move(const char* usermove)
{

	quit_search_safe();

	if(p.is_algeb_move_legal((char*)usermove))
	{
		game[game_ptr++]=p;
		p.make_move(p.try_move);
		
		analyze_pos();
	}
}

void set_board(const char* fen)
{

	quit_search_safe();

	game_ptr=0;
	p.set_from_fen((char*)fen);

	analyze_pos();
}