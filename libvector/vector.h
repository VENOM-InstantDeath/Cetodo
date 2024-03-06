#include <stddef.h>
#ifndef VECTOR_H
#define VECTOR_H
typedef enum {
	NULL_STR,
	NULL_SOBJ,
	NULL_OBJ,
	OUT_OF_BOUNDS,
	VECTYPE_ERROR
} vector_exception;
typedef enum {
	VINT,
	VGEN,
	VSTRLIST
} vectype;

typedef struct {
	char* c_str;
	size_t memsize;
} string;
typedef struct {
    union {
		int* num;
		void** gen;
		char** strls;
	} T;
	vectype type;
    size_t memsize;
} vector;

void* _realloc(void* ptr, size_t size);
void* _realloc2(void* ptr, size_t size);

void vector_raise(char* function, vector_exception exception);

string string_init(char* str);
char string_get_at(string* S, int n);
void string_asign_at(string* S, int n, char ch);
char* string_get_c_str(string* S);
int string_count(string* S, char ch);
void string_append(string* S, char* str);
void string_append_int(string* S, int num);
void string_append_char(string* S, char ch);
int string_find(string* S, char ch);
int string_rfind(string* S, char ch);
void string_append_fmt(string* S, char* fmt, ...);
void string_free(string* S);
vector string_split(string* S, char sep);

/*isdigit
islower
isupper
tolower
toupper
toint*/

vector vector_init(vectype T);

int vector_get_int_at(vector* V, int n);
void* vector_get_generic_at(vector* V, int n);
char* vector_get_string_at(vector* V, int n);
#define vector_get_at(V, n) \
V->type == VINT ? vector_get_int_at(V, n)\
:V->type == VGEN ? vector_get_generic_at(V, n)\
:V->type == VSTRLIST ? vector_get_string_at(V, n) : 0

void vector_asign_int_at(vector* V, int n, int arg);
void vector_asign_generic_at(vector* V, int n, void* arg);
void vector_asign_string_at(vector* V, int n, char* arg);
#define vector_asign_at(V, n, arg) \
switch(V->type) {\
	case VINT: vector_asign_int_at(V, n, arg);break;\
	case VGEN: vector_asign_generic_at(V, n, arg);break;\
	case VSTRLIST:vector_asign_string_at(V, n, arg);break;\
}

int vector_count_int(vector* V, int n);
int vector_count_string(vector* V, char* str);
#define vector_count(V, n) \
switch(V->type) {\
	case VINT: vector_count_int(V, n);break;\
	case VGEN: vector_raise("vector_count", VECTYPE_ERROR);break;\
	case VSTRLIST:vector_count_string(V, n);break;\
}

void vector_append_int(vector* V, int arg);
void vector_append_generic(vector* V, void* arg);
void vector_append_string(vector* V, char* str);
#define vector_append(V, arg) \
switch(V->type) {\
	case VINT: vector_append_int(V, arg);break;\
	case VGEN: vector_append_generic(V, arg);break;\
	case VSTRLIST:vector_append_string(V, arg);break;\
}

int vector_find_int(vector* V, int n);
int vector_find_string(vector* V, char* str);
#define vector_find(V, n) \
switch(V->type) {\
	case VINT: vector_find_int(V, n);break;\
	case VSTRLIST:vector_find_string(V, n);break;\
}

int vector_rfind_int(vector* V, int n);
int vector_rfind_string(vector* V, char* str);
#define vector_rfind(V, n) \
switch(V->type) {\
	case VINT: vector_rfind_int(V, n);break;\
	case VSTRLIST:vector_rfind_string(V, n);break;\
}

void vector_remove(vector* V, int index);

void vector_insert_int(vector* V, int arg);
void vector_insert_generic(vector* V, void* arg);
void vector_insert_string(vector* V, char* arg);
#define vector_insert(V, n) \
switch(V->type) {\
	case VINT: vector_insert_int(V, n);break;\
	case VGEN: vector_insert_generic(V, n);break;\
	case VSTRLIST:vector_insert_string(V, n);break;\
}
#endif
