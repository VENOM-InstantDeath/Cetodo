#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <sys/stat.h>
#include "libvector/vector.h"
#include "libncread/ncread.h"

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
	vector func;
	int size;
};
struct Menuopt {
	vector optname;
	struct Callback callback;
	struct Keybinding keybinding;
	int cursor[2];
};

struct Menuopt menuopt_init() {
	struct Menuopt m = {vector_init(VSTRLIST), {NULL, 0}, {NULL, NULL, 0}, 0};
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
		free(buffer);
	}
	string_free(&path); string_free(&s_path_data);
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
	string_free(&path); string_free(&s_path_trusted);
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
		string_free(&num);
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
		string_free(&line);
	}
	wrefresh(stdscr);
	wrefresh(task_win);
}
int menu(WINDOW* win, struct Menuopt* menuopt, void** data) {
	int y, x; getmaxyx(win, y, x);
	wattron(win, COLOR_PAIR(WHITE_BLACK));
	mvwaddstr(win, menuopt->cursor[0], 4, vector_get_string_at(&menuopt->optname, menuopt->cursor[1]));
	wattroff(win, COLOR_PAIR(WHITE_BLACK));
	for (;;) {
		int ch = wgetch(win);
		switch (ch) {
			case 27:
				return 0;
			case KEY_UP:
				if (!menuopt->cursor[1]) continue;
				mvwaddstr(win, menuopt->cursor[0], 4, vector_get_string_at(&menuopt->optname, menuopt->cursor[1]));
				menuopt->cursor[0]--; menuopt->cursor[1]--;
				wattron(win, COLOR_PAIR(WHITE_BLACK));
				mvwaddstr(win, menuopt->cursor[0], 4, vector_get_string_at(&menuopt->optname, menuopt->cursor[1]));
				wattroff(win, COLOR_PAIR(WHITE_BLACK));
				break;
			case KEY_DOWN:
				if (menuopt->cursor[1] == menuopt->callback.size-1) continue;
				mvwaddstr(win, menuopt->cursor[0], 4, vector_get_string_at(&menuopt->optname, menuopt->cursor[1]));
				menuopt->cursor[0]++; menuopt->cursor[1]++;
				wattron(win, COLOR_PAIR(WHITE_BLACK));
				mvwaddstr(win, menuopt->cursor[0], 4, vector_get_string_at(&menuopt->optname, menuopt->cursor[1]));
				wattroff(win, COLOR_PAIR(WHITE_BLACK));
				break;
			case 10:
				return ((functype)vector_get_generic_at(&menuopt->callback.func, menuopt->cursor[1]))(win, menuopt, data);
				break;
			default:
				for (int i=0; i<menuopt->keybinding.size; i++) {
					if (ch == menuopt->keybinding.key[i]) {
						return menuopt->keybinding.func[i](win, menuopt, data);
					}
				}
		}
	}
}

vector get_listnames(vector* Lists) {
	vector optname = vector_init(VSTRLIST);
	for (int i=0; i<Lists->memsize; i++) {
		struct List* l = vector_get_generic_at(Lists, i);
		vector_append_string(&optname, l->name);
	}
	return optname;
}

vector get_tasknames(struct List* L) {
	vector optname = vector_init(VSTRLIST);
	for (int i=0; i<L->tasks.memsize; i++) {
		struct List* l = vector_get_generic_at(&L->tasks, i);
		vector_append_string(&optname, l->name);
	}
	return optname;
}

int task_check(WINDOW* win, struct Menuopt* menuopt, void** data) {
	WINDOW* task_win = data[0];
	vector* Tasks = data[1];
	struct Task* T = vector_get_generic_at(Tasks, menuopt->cursor[1]);
	mvwaddch(task_win, menuopt->cursor[0], 1, T->status ? ' ' : 'x');
	wrefresh(task_win);
	T->status = !T->status;
	return 1;
}

char* ask_task_name(WINDOW* task_win) {
	int y, x; getmaxyx(stdscr, y, x);
	WINDOW* win = newwin(4, 30, y/2-2, x/2-12); 
	keypad(win, 1);
	getmaxyx(win, y, x);
	mvwaddstr(win, 0, x/2-6, "Add Task");
	mvwaddstr(win, 2, 2, "Nombre:");
	char* name=NULL;
	for (;;) {
		if (!Sread(win, 2, 10, 15, 20, &name) || !name || !strlen(name)) continue;
		break;
	}
	delwin(win);
	wrefresh(task_win);
	return name;
}

struct Task* new_task(char* task_name) {
	struct Task* T = malloc(sizeof(struct Task));
	T->name=task_name; T->status=0;
}

int add_task(WINDOW* win, struct Menuopt* menuopt, void** data) {
	WINDOW* task_win = data[0];
	vector* Tasks = data[1];
	char* task_name = ask_task_name(task_win);
	struct Task* T = new_task(task_name); vector_append_generic(Tasks, T);
	vector_append_string(&menuopt->optname, task_name);

	int y,x; getmaxyx(task_win, y,x);
	if (menuopt->callback.size<y) {
		string line = string_init("[ ] ");string_append(&line, task_name);
		mvwaddstr(task_win, menuopt->callback.size, 0, string_get_c_str(&line));
		string_free(&line);
	}
	menuopt->callback.size++;
	vector_append_generic(&menuopt->callback.func, task_check);
	return 1;
}

int delete_task(WINDOW* win, struct Menuopt* menuopt, void** data) {
	WINDOW* task_win = data[0];
	vector* Tasks = data[1];
	if (!menuopt->callback.size) return 1;
	vector_remove(Tasks, menuopt->cursor[1]);
	vector_remove(&menuopt->optname, menuopt->cursor[1]);
	vector_remove(&menuopt->callback.func, menuopt->cursor[1]);
}

int go_back(WINDOW* win, struct Menuopt* menuopt, void** data) {return 0;}

int open_tasklist(WINDOW* win, struct Menuopt* menuopt, void** data) {
	WINDOW* task_win = data[0];
	vector* Lists = data[1];
	struct List* L = vector_get_generic_at(Lists, menuopt->cursor[1]);

	draw_tasks(task_win, L);
	vector optname = get_tasknames(L);
	vector funcs = vector_init(VGEN);
	for (int i=0; i<L->tasks.memsize; i++) vector_asign_generic_at(&funcs, i, task_check);
	char keys[] = {'a', 's'};
	functype kbinds[] = {add_task, go_back};
	struct Menuopt _menuopt = {optname, {funcs, L->tasks.memsize}, {keys, kbinds, 2}, {0, 0}};
	void* _data[2] = {task_win, &L->tasks};
	for (;;) {
		if (menu(task_win, &_menuopt, _data)) {
			touchwin(task_win);wrefresh(task_win);
		} else {
		vector_free(&_menuopt.callback.func);
			return 1;
		}
	}
}

int main() {
	WINDOW* stdscr = initscr();
	start_color(); use_default_colors();
	keypad(stdscr, 1); set_escdelay(200);
	curs_set(0); noecho(); cbreak();

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

	vector optname = get_listnames(&Lists);
	vector funcs = vector_init(VGEN);
	for (int i=0; i<Lists.memsize; i++) vector_asign_generic_at(&funcs, i, open_tasklist);
	struct Menuopt menuopt = {optname, {funcs, Lists.memsize}, {0,0,0}, {0, 0}};
	void* data[2] = {task_win, &Lists};
	for (;;) {
		if (menu(tasklist_win, &menuopt, data)) {
			touchwin(tasklist_win);wrefresh(tasklist_win);
		} else {
			break;
		}
	}
	endwin();
}
