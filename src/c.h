#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define NELEMS(a) ((int)(sizeof (a) / sizeof ((a)[0])))
#define roundup(x, n) (((x) + ((n) - 1)) & (~((n) - 1)))
#define NEW(p, a) ((p) = allocate(sizeof *(p), (a)))


enum {
    PERM = 0,
    FUNC,
    STMT
};


typedef struct coord {
    char *file;
    unsigned x, y;
} Coordinate;


// extern var
extern char *first_file;
extern char *file;
extern Coordinate src;


// extern func declare
extern void error(const char *, ...);
extern void fprint(FILE *f, const char *fmt, ...);
extern void vfprint(FILE *, char *, const char *, va_list);


// string.c
extern char *copy_str(char *str);
extern char *copy_sub_str(char *str, unsigned start, unsigned end);
extern char *copy_first_length_str(char *str, unsigned len);
extern char *decimal_num_to_str(int n);


// alloc.c
extern void *allocate(unsigned long n, unsigned arena_no);
extern void *new_array(unsigned long m, unsigned long n, unsigned arena_no);
extern void deallocate(unsigned arena_no);

