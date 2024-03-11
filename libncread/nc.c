#include <ncurses.h>
#include "nc.h"
#include "../libvector/vector.h"

void string_append_char_at(string* S, int index, char ch) {
	size_t end = S->memsize;
	string_grow(S, 1);
	index = index < 0 ? S->memsize+index : index;
	if (S->memsize > 1) string_shift(S, index, end, 1);
	string_asign_at(S, index, ch);
}

void string_pop_at(string* S, int index) {
	index = index < 0 ? S->memsize+index : index;
	if (S->memsize > 1) string_shift_left(S, index+1, S->memsize, 1);
	string_shrink(S, 1);
}

void clear_region(WINDOW* win, int y, int x, int n) {
	for (;x<n; x++) {mvwaddch(win, y, x, ' ');}
}

int Sread(WINDOW* win, int y, int x, int visual_limit, int char_limit, char** buff) {
	string S = string_init(*buff);
	curs_set(1);
	int p[2]={0};
	int scrolln=0;
	for (;;) {
		int ch = mvwgetch(win, y, x+p[0]);
		switch (ch) {
			case 27:
				curs_set(0);
				return 0;
			case KEY_LEFT:
				if (!p[1]) continue;
				if (!p[0]) {
					clear_region(win, y, x, x+visual_limit);
					scrolln--;
					mvwaddnstr(win, y, x, S.c_str+scrolln, visual_limit);
					p[1]--;
					continue;
				}
				p[0]--; p[1]--;
				break;
			case KEY_RIGHT:
				if (p[1] == S.memsize || p[1]==char_limit) continue;
				if (p[0] == visual_limit) {
					clear_region(win, y, x, x+visual_limit);
					scrolln++;
					mvwaddnstr(win, y, x, S.c_str+scrolln, visual_limit);
					p[1]++;
					continue;
				}
				p[0]++; p[1]++;
				break;
			case 127:
			case KEY_BACKSPACE:
				if (!p[1]) continue;	
				string_pop_at(&S, p[1]-1);
				if (!p[0]) {
					clear_region(win, y, x, x+visual_limit);
					scrolln--;
					mvwaddnstr(win, y, x, S.c_str+scrolln, visual_limit);
					p[1]--;
				} else {
					p[0]--; p[1]--;	
					clear_region(win, y, x+p[0], x+visual_limit);
					mvwaddnstr(win, y, x+p[0], S.c_str+p[1], S.memsize>visual_limit ? visual_limit+scrolln-p[1] : S.memsize-p[1]);
				}
				break;
			case 4:
			case 10:
				curs_set(0);
				*buff = string_get_c_str(&S);
				return 1;
			default:
				if (S.memsize == char_limit) continue; // IF p[0] IS VISUAL_LIMIT SCROLL HORIZONTALLY
				if (!((ch >= ' ' && ch <= '~') || (ch >= 225 && ch <= 250))) continue;
				string_append_char_at(&S, p[1],ch);
				if (p[0] == visual_limit) {
					clear_region(win, y, x, x+visual_limit);
					scrolln++;
					mvwaddnstr(win, y, x, S.c_str+scrolln, visual_limit);
					p[1]++;
					continue;
				} else {
					clear_region(win, y, x+p[0], x+visual_limit);
					mvwaddnstr(win, y, x+p[0], S.c_str+p[1], S.memsize>visual_limit ? visual_limit+scrolln-p[1] : S.memsize-p[1]);
					p[0]++; p[1]++;
				}
		}
	}
	return 1;
}
