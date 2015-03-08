#include "simplemente.h"
#include "xboard.h"

#include "position.h"

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <process.h>

using namespace std;

Position p;

Position game[200];
int game_ptr=0;

void search_thread(void* param)
{
	p.search(*((int*)param));
}

#ifndef XBOARD_COMPATIBLE
char buf[100];
void main(int argc,char** argv)
{

	init_move_table();

	p.reset();
	p.print();

	do
	{

		cin.getline(buf,100);
		
		if(buf[0]=='p')
		{
			p.print();
		}

		if(buf[0]=='r')
		{
			p.reset();
			p.print();
			game_ptr=0;
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
				int search_depth=buf[1]-'0';
				quit_search=false;
				_beginthread(search_thread,0,&search_depth);
			}
		}

		if(buf[0]=='q')
		{
			quit_search=true;
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

void init_main()
{
	init_move_table();
	reset_game();
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