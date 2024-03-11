#include <ncurses.h>
#include "nc.h"

int main() {
	WINDOW* stdscr = initscr();
	curs_set(0); keypad(stdscr, 1);
	start_color(); use_default_colors();
	cbreak(); noecho();
	init_pair(1, 20, 15);
	int y, x; getmaxyx(stdscr, y, x);
	WINDOW* win = newwin(3, 50, y/2-1, x/2-25);
	keypad(win, 1);
	wbkgd(win, COLOR_PAIR(1));
	wrefresh(stdscr); wrefresh(win);
	mvwaddch(win, 1, 23, 'x');
	char* buff=NULL;
	int err = Sread(win, 1, 2, 20, 30, &buff); 
	endwin();
	if (err) puts(buff);
}
