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

char buf[100];
void main_old(int argc,char** argv)
{

	init_move_table();

	p.reset();
	//p.print();

	ofstream outfile;
	outfile.open("test.txt");
	outfile << "test:" << endl;
	outfile.close();

	do
	{

		cin.getline(buf,100);
		outfile.open("test.txt",ios_base::app);
		outfile << buf << endl;
		outfile.close();
		

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

/////////////////////////////////////////////////////////////

void reset_game()
{
	p.reset();
	game_ptr=0;
	analyze_mode=false;
}

void init_main()
{
	init_move_table();
	reset_game();
}

void quit_search_safe()
{
	quit_search=true;
	while(!search_quitted)
	{
		Sleep(50);
	}
	quit_search=false;
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

void analyze_move(const char* usermove)
{

	do_make_move=false;

	quit_search_safe();

	search_depth=10;
	if(p.is_algeb_move_legal((char*)usermove))
	{
		game[game_ptr++]=p;
		p.make_move(p.try_move);
		_beginthread(search_thread,0,&search_depth);
	}
}