#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <json.h>
#include <sys/stat.h>
#include "libvector/vector.h"

enum COLORS {
	WHITE_BLACK=1,
	WHITE_BLUE
};

struct Menuopt;
typedef int (*functype)(WINDOW*, struct Menuopt*, void**);

struct Task {
	char* name;
	int status;
};
struct List {
	char* name;
	vector tasks;
};
struct Keybinding {
	char* key;
	functype* func;
	int size;
};
struct Callback {
	functype* func;
	int size;
};
struct Menuopt {
	char** optname;
	struct Callback callback;
	struct Keybinding keybinding;
	int cursor[2];
};

struct Menuopt menuopt_init() {
	struct Menuopt m = {NULL, {NULL, 0}, {NULL, NULL, 0}, 0};
	return m;
}


vector load_data_file() {
	struct stat st;
	vector Lists = vector_init(VGEN);
	char* HOME = getenv("HOME");
	string path = string_init(HOME);
	string_append(&path, "/.local");
	mkdir(string_get_c_str(&path), 0755);
	string_append(&path, "/share");
	mkdir(string_get_c_str(&path), 0755);
	string_append(&path, "/etodo/");
	char* cpath = string_get_c_str(&path);
	mkdir(cpath, 0755);
	string s_path_data = string_init(cpath);
	string_append(&s_path_data, "data.json");
	char* path_data = string_get_c_str(&s_path_data);
	if (stat(path_data, &st) == -1) {
		FILE* F = fopen(path_data, "w");
		fputs("{}\n", F);
		fclose(F);
	} else {
		FILE* F = fopen(path_data, "r");
		char* buffer = malloc(st.st_size+1);
		buffer[st.st_size] = 0;
		fread(buffer, 1, st.st_size, F);
		fclose(F);
		json_object* jobj = json_tokener_parse(buffer);
		json_object_object_foreach(jobj, key, val) {
			struct List* list = malloc(sizeof(struct List));
			list->name = key;
			list->tasks = vector_init(VGEN);
			json_object_object_foreach(val, k, v) {
				struct Task* task = malloc(sizeof(struct Task));
				task->name = k;
				task->status = json_object_get_int(v);
				vector_append_generic(&list->tasks, task);
			}
			vector_append_generic(&Lists, list);
		}
	}
	return Lists;
}
void load_trusted_file() {
	struct stat st;
	char* HOME = getenv("HOME");
	string path = string_init(HOME);
	string_append(&path, "/.local");
	mkdir(string_get_c_str(&path), 0755);
	string_append(&path, "/share");
	mkdir(string_get_c_str(&path), 0755);
	string_append(&path, "/etodo/");
	char* cpath = string_get_c_str(&path);
	mkdir(cpath, 0755);
	string s_path_trusted = string_init(cpath);
	string_append(&s_path_trusted, "trusted");
	char* path_trusted = string_get_c_str(&s_path_trusted);
	if (stat(path_trusted, &st) == -1) {
	} else {
	}
}

WINDOW* draw_upper_bar(WINDOW* stdscr, int y, int x) {
	WINDOW* upper_bar = newwin(1, x, 0, 0);
	wbkgd(upper_bar, COLOR_PAIR(WHITE_BLACK));
	mvwaddstr(upper_bar, 0, 2, "ETODO");
	mvwaddstr(upper_bar, 0, x/2-8, "Listas de tareas");
	wrefresh(stdscr);
	wrefresh(upper_bar);
	return upper_bar;
}
WINDOW* draw_lower_bar(WINDOW* stdscr, int y, int x) {
	WINDOW* lower_bar = newwin(1, x, y-1, 0);
	wattron(lower_bar, COLOR_PAIR(WHITE_BLACK));
	mvwaddstr(lower_bar, 0, 0, " ? ");
	wattroff(lower_bar, COLOR_PAIR(WHITE_BLACK));
	mvwaddstr(lower_bar, 0, 4, "Ayuda");
	wrefresh(stdscr);
	wrefresh(lower_bar);
	return lower_bar;
}
WINDOW* draw_tasklists(WINDOW* stdscr, int y, int x, vector* Lists) {
	WINDOW* tasklist_win = newwin(y-3, x, 2, 0);
	keypad(tasklist_win, 1); scrollok(tasklist_win, 1);
	for (int i=0; i<Lists->memsize && i<y-3; i++) {
		struct List* L = vector_get_generic_at(Lists, i);
		string num = string_init(NULL); string_append_int(&num, L->tasks.memsize);
		mvwaddstr(tasklist_win, i, 0, string_get_c_str(&num));
		mvwaddstr(tasklist_win, i, 4, L->name);
	}
	wrefresh(stdscr);
	wrefresh(tasklist_win);
	return tasklist_win;
}
void draw_tasks(WINDOW* task_win, struct List* L) {
	int y, x; getmaxyx(task_win, y, x);
	wmove(task_win, 0, 0); wclrtobot(task_win);
	for (int i=0; i<L->tasks.memsize && i<y; i++) {
		struct Task* T = vector_get_generic_at(&L->tasks, i);
		string line = string_init(NULL); string_append_fmt(&line, "[%c] %s", T->status ? 'x' : ' ', T->name);
		mvwaddstr(task_win, i, 0, string_get_c_str(&line));
	}
	wrefresh(stdscr);
	wrefresh(task_win);
}
int menu(WINDOW* win, struct Menuopt* menuopt, void** data) {
	int y, x; getmaxyx(win, y, x);
	wattron(win, COLOR_PAIR(WHITE_BLACK));
	mvwaddstr(win, menuopt->cursor[0], 4, menuopt->optname[menuopt->cursor[1]]);
	wattroff(win, COLOR_PAIR(WHITE_BLACK));
	for (;;) {
		int ch = wgetch(win);
		switch (ch) {
			case 27:
				return 0;
			case KEY_UP:
				if (!menuopt->cursor[1]) continue;
				mvwaddstr(win, menuopt->cursor[0], 4, menuopt->optname[menuopt->cursor[1]]);
				menuopt->cursor[0]--; menuopt->cursor[1]--;
				wattron(win, COLOR_PAIR(WHITE_BLACK));
				mvwaddstr(win, menuopt->cursor[0], 4, menuopt->optname[menuopt->cursor[1]]);
				wattroff(win, COLOR_PAIR(WHITE_BLACK));
				break;
			case KEY_DOWN:
				if (menuopt->cursor[1] == menuopt->callback.size-1) continue;
				mvwaddstr(win, menuopt->cursor[0], 4, menuopt->optname[menuopt->cursor[1]]);
				menuopt->cursor[0]++; menuopt->cursor[1]++;
				wattron(win, COLOR_PAIR(WHITE_BLACK));
				mvwaddstr(win, menuopt->cursor[0], 4, menuopt->optname[menuopt->cursor[1]]);
				wattroff(win, COLOR_PAIR(WHITE_BLACK));
				break;
			case 10:
				data[2] = vector_get_generic_at((vector*)data[1], menuopt->cursor[1]);
				return menuopt->callback.func[menuopt->cursor[1]](win, menuopt, data);
				break;
			default:
				break;
		}
	}
}

char** get_listnames(vector* Lists) {
	char** optname = malloc(sizeof(char*)*Lists->memsize);
	for (int i=0; i<Lists->memsize; i++) {
		struct List* l = vector_get_generic_at(Lists, i);
		optname[i] = l->name;
	}
	return optname;
}

char** get_tasknames(struct List* L) {
	char** optname = malloc(sizeof(char*)*L->tasks.memsize);
	for (int i=0; i<L->tasks.memsize; i++) {
		struct List* l = vector_get_generic_at(&L->tasks, i);
		optname[i] = l->name;
	}
	return optname;
}

int add_task(WINDOW* win, struct Menuopt* menuopt, void** data) {
	return 1;
}

int open_tasklist(WINDOW* win, struct Menuopt* menuopt, void** data) {
	WINDOW* task_win = data[0];
	vector* Lists = data[1];
	struct List* L = data[2];

	draw_tasks(task_win, L);
	char** optname = get_tasknames(L);
	struct Menuopt _menuopt = {optname, {0, L->tasks.memsize}, {0, 0, 0}, {0, 0}};
	menu(task_win, &_menuopt, 0);

	return 1;
}

int main() {
	WINDOW* stdscr = initscr();
	start_color(); use_default_colors();
	keypad(stdscr, 1); set_escdelay(200);
	curs_set(0); noecho();

	init_pair(WHITE_BLACK, 0, 15);
	init_pair(WHITE_BLUE, 15, 20);

	int y,x; getmaxyx(stdscr, y,x);

	vector Lists = load_data_file();

	WINDOW* upper_bar = draw_upper_bar(stdscr, y, x);
	WINDOW* tasklist_win = draw_tasklists(stdscr, y, x, &Lists);
	WINDOW* task_win = newwin(y-3, x, 2, 0);
	WINDOW* lower_bar = draw_lower_bar(stdscr, y, x);
	keypad(task_win, 1); scrollok(task_win, 1);
	wrefresh(stdscr);

	char** optname = get_listnames(&Lists);
	functype* funcs = malloc(sizeof(functype)*Lists.memsize);
	for (int i=0; i<Lists.memsize; i++) funcs[i] = open_tasklist;
	struct Menuopt menuopt = {optname, {funcs, Lists.memsize}, {0,0,0}, {0, 0}};
	void* data[3] = {task_win, &Lists};
	menu(tasklist_win, &menuopt, data);

	endwin();
}
