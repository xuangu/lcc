#include "c.h"

struct block
{
    struct block *next;
    char *avail;
    char *limit;
};

union align
{
    long l;
    char *p;
    double d;
    int (*f)(void);
};

union header {
    struct block *block;
    union align align;
};

// first[n] 始终指向第n个分配区链表的表头，在释放第n个分配区时，用来从头追踪
static struct block first[3] = { {NULL}, {NULL}, {NULL} };
// arena[n] 始终指向第n个分配区链表的表尾
static struct block *arena[3] = { &first[0], &first[1], &first[2] };
static struct block *free_blocks = NULL;

#define BUFFER_SIZE (10 * 1024)

void *allocate(unsigned long n, unsigned arena_no) {
    asset(n < NELEMS(arena));
    asset(n > 0);

    struct block *pb = arena[arena_no];
    n = roundup(n, sizeof (union align));

    // new memory block
    while (pb->limit - pb->avail < n)
    {
        if ((pb->next = free_blocks) != NULL) 
        {
            free_blocks = free_blocks->next;
            pb = pb->next;
        }
        else 
        {
            unsigned long size = sizeof(union header) + n + roundup(BUFFER_SIZE, sizeof(union align));
            pb->next = malloc(size);
            pb = pb->next;
            if (pb == NULL) 
            {
                error("insufficient memory");
                exit(1);
            }
            pb->limit = (char *)pb + size;
        }
        pb->avail = (char *)(sizeof(union header *)pb + 1);
        pb->next = NULL;
        arena[arena_no] = pb;
    }

    pb->avail += n;
    return pb->avail - n;
}

void *new_array(unsigned long m, unsigned long n, unsigned arena_no) {
    return allocate(m * n, arena_no);
}

void deallocate(unsigned arena_no) {
    asset(arena_no < NELEMS(arena));
    arena[arena_no]->next = free_blocks;
    free_blocks = first[arena_no].next;
    first[arena_no].next = NULL;
    arena[arena_no] = &first[arena_no];
}