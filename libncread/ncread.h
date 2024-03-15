#ifndef NCREAD_H
#define NCREAD_H
#include <ncurses.h>
#include "../libvector/vector.h"
void string_append_char_at(string* S, int index, char ch);
int Sread(WINDOW* win, int y, int x, int visual_limit, int char_limit, char** buff);
#endif
