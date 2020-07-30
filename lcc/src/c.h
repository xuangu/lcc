#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include "token.h"

#define NELEMS(a) ((int)(sizeof (a) / sizeof ((a)[0])))
#define roundup(x, n) (((x) + ((n) - 1)) & (~((n) - 1)))
#define NEW(p, a) ((p) = allocate(sizeof *(p), (a)))
#define NEW0(p, a) memset((NEW((p), (a))), 0, sizeof (*p))

enum {
    PERM = 0,
    FUNC,
    STMT
};

typedef struct coord {
    char *file;
    unsigned x, y;
} Coordinate;

typedef struct symbol *Symbol;

typedef struct list *List;

typedef struct type *Type;

// symbol.c
typedef struct symbol_table *SymbolTable;

// 存放常量符号的常量值
typedef union value {
    char signed_char_val;
    short signed_short_val;
    int signed_int_val;
    unsigned char unsigned_char_val;
    unsigned short unsigned_short_val;
    unsigned int unsigned_int_val;
    float float_val;
    double double_val;
    void *pointer_val;
} Value;

struct symbol {
    char *name;
    // 符号的作用域层级编号
    int scope;
    // 符号定义点的位置
    Coordinate src;
    // up域将符号表中的所有符号链成一个链表，最后插入的符号位表头，通过向后遍历可以遍历当前作用域下的所有符号，包括因为嵌套内层作用域标识符声明而被隐藏的符号
    Symbol up;
    // 该域指向一个Coordinate链表，保存每个符号的所有使用信息。
    List uses;
    // 符号的存储类型，可以为auto, register, static, extern, enum, typedef，对常量和标号类的符号，不使用该域
    int storage_class;
    // flag fields，TODO:
    // 表示由lcc内部产生的一个符号，标号符号，该域置1
    unsigned generated: 1;
    // 临时变量
    unsigned temporary: 1;
    // TODO:
    unsigned addressed: 1;
    // TODO: constant等符号该域置1
    unsigned defined: 1;
    
    
    // 符号类型信息
    Type type;
    // 保存变量和标号的粗略引用次数
    float ref;
    // 为标号、结构和联合类型、枚举标识符、枚举类型、常量、函数、全局和静态变量、临时变量等提供附加信息
    union {
        // 存放”标号“符号附加信息
        struct {
            // ”标号“符号内部表示唯一值
            int val;
            // 如果两个或更多的内部标号指向相同的位置，则用equated_to指向其中的一个标号
            Symbol equated_to;
        } label;
        
        struct {
            Value val;
            Symbol loc;
        } constant;
        
    } extend_info;
    
};

// 类型结构，记录类型信息
struct type {
    // 操作符编码
    int op;
    // 操作数
    Type type;
    // 类型最小对齐字节
    int align;
    // 类型大小
    int size;
    // TODO:
    union {
        Symbol sym;
    } ex_info;
};

enum { CONSTANTS = 1, LABELS, GLOBAL, PARAM, LOCAL };

extern SymbolTable constants;
extern SymbolTable externals;
// 具有文件作用域的标识符，globals变量时刻指向当前插入的符号表的链表头
extern SymbolTable globals;
// 一般标识符
extern SymbolTable identifiers;
// 内部标号
extern SymbolTable labels;
// types表，包含了用标识符或标记命名的所有类型
extern SymbolTable types;
// 追踪当前作用域的全局变量，level的值（上述定义的诸如CONSTANTS、GLOBAL等值）和对应的符号表（上述定义的符号表），共同表示一个作用域
extern int level;

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


// types.c
// 删除types符号表中所有大于参数lev的类型符号
extern void rm_sym_from_types_table(int lev);
extern bool is_qualified(Type ty);
// 返回限定类型的非限定操作数
extern Type unqualify(Type ty);
extern bool equal_type(Type ty1, Type ty2, int arr_empty_ret_val);
extern char *get_const_name(Type ty, Value val);
