
#ifndef SIMPLEMENTE_H
#define SIMPLEMENTE_H
#endif

//#define QUIESCENCE_SEARCH
//#define CALC_LINE

extern void set_board(const char*);
extern void reset_game();
extern void analyze_move(const char*);
extern void analyze_and_make_move(const char*);
extern void just_make_a_move();
extern void force();
extern void undo_move();
extern void init_main();
extern void quit_search_safe();