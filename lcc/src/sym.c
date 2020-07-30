//
//  symbol.c
//  lcc
//
//  Created by Gu,Xuan on 2020/7/27.
//  Copyright © 2020 Gu,Xuan. All rights reserved.
//

#include "c.h"
// 如下语法也是合法的，struct的声明具有文件作用域，而不是块作用域
//struct table {
//    int level;
//    Table previous;
//    struct entry {
//        struct symbol sym;
//        struct entry *link;
//    } *buckets[256];
//    Symbol all;
//};

// entry实现了一个链式hash表，每个链表元素保存了一个symbol结构，和hash链中下一个入口的指针
struct entry {
    struct symbol sym;
    struct entry *link;
};

// 符号表，根据作用域将某个作用域内的符号保存在一个哈希表中
struct symbol_table {
    // 作用域
    int scope_level;
    // 外层作用域
    SymbolTable outer;
    struct entry *buckets[256];
    // 该域指向当前及其外层作用域中所有符号组成的链表的表头，该链表通过symbol结构的up域链接
    Symbol header_sym;
};

static int get_sym_name_hash_value(char *name);
static bool is_const_equal(Type ty, struct entry *entry, Value val);

#define HASH_SIZE NELEMS(((SymbolTable)0)->buckets)

static struct symbol_table s_constants = { CONSTANTS };
static struct symbol_table s_externals = { GLOBAL };
static struct symbol_table s_identifiers = { GLOBAL };
static struct symbol_table s_types = { GLOBAL };

SymbolTable constants = &s_constants;
SymbolTable externals = &s_externals;
SymbolTable identifiers = &s_identifiers;
SymbolTable globals = &s_identifiers;
SymbolTable types = &s_types;
SymbolTable labels = NULL;

int level = GLOBAL;

SymbolTable alloc_table(SymbolTable table, int level) {
    SymbolTable tb;
    NEW0(tb, FUNC);
    
    tb->outer = table;
    tb->scope_level = level;
    
    if (table) {
        tb->header_sym = table->header_sym;
    }
    
    return table;
}

void foreach(SymbolTable table,
             int level,
             void (*apply)(Symbol, void *),
             void *lambda_data) {
    while (table->scope_level > level) {
        table = table->outer;
    }
    
    if (table && table->scope_level == level) {
        Symbol sym = NULL;
        // 保留进入循环的src现场
        Coordinate save = src;
        
        for (sym = table->header_sym; sym->scope == level; sym = sym->up) {
            src = sym->src;
            (*apply)(sym, lambda_data);
        }
        // 恢复src现场
        src = save;
    }
}

void enter_scope() {
    ++level;
}

void exit_scope() {
    rm_sym_from_types_table(level);
    
    if (types->scope_level == level) {
        types = types->outer;
    }
    
    if (identifiers->scope_level == level) {
        identifiers = identifiers->outer;
    }
    
    assert(level > GLOBAL);
    
    --level;
}


/**
 为给定的name生成一个符号，并把该符号加入到与给定作用域层次相对应的符号表中，如果给定的level值大于给定符号表的作用域层次，则新建一个符号表，该函数返回指向生成的符号的指针

 @param name 给定的符号name
 @param ptable 给定的符号表
 @param level 给定作用域层次
 @param arena 分配新符号的存储区
 @return 返回创建的符号指针
 */
Symbol install(char *name, SymbolTable *ptable, int level, int arena) {
    SymbolTable table = *ptable;
    struct entry *entry;
    
    if (table->scope_level < level && level > 0) {
        SymbolTable new_table = alloc_table(table, level);
        *ptable = new_table;
        table = new_table;
    }
    
    NEW0(entry, arena);
    entry->sym.name = name;
    entry->sym.scope = level;
    entry->sym.up = table->header_sym;
    table->header_sym = &entry->sym;
    
    int hash = get_sym_name_hash_value(name);
    entry->link = table->buckets[hash];
    table->buckets[hash] = entry;
    
    return &entry->sym;
}


/**
 在符号表中查找一个name

 @param name 待查找的name
 @param table 给定的符号表
 @return 返回指向name符号的指针
 */
Symbol lookup(char *name, SymbolTable table) {
    struct entry *entry;
    int hash = get_sym_name_hash_value(name);
    
    do {
        for (entry = table->buckets[hash]; entry; entry = entry->link) {
            if (name == entry->sym.name) {
                return &entry->sym;
            }
        }
    } while ((table = table->outer) != NULL);
    
    return NULL;
}

static int get_sym_name_hash_value(char *name) {
    return (long)name & (HASH_SIZE - 1);
}

/**
 通过累加计数器产生一个整数，用来在lcc内部唯一表示 编译器产生的内部标号和源程序中的标号

 @param n TODO:
 @return TODO:
 */
int gen_label(int n) {
    static int label = 1;
    
    label += n;
    
    return label - n;
}

/// 源程序中的每个标号都有一个内部标号，这些内部标号和编译器产生的其它标号都保存在labels表中。对于每个函数都会建立一个这样的表，并有find_labels函数进行管理。
/// find_label函数的输入参数是一个标号数，返回该标号对应的符号，如果需要，会建立该符号，进行初始化，并通知编译后端
/// @param label 标号数
Symbol find_label(int label) {
    struct entry *entry;
    
    int hash = label & (HASH_SIZE - 1);
    
    for (entry = labels->buckets[hash]; entry; entry = entry->link) {
        if (entry->sym.extend_info.label.val == label) {
            return &entry->sym;
        }
    }
    
    NEW0(entry, FUNC);
    entry->sym.extend_info.label.val = label;
    entry->sym.scope = LABELS;
    entry->sym.name = decimal_num_to_str(label);
    entry->sym.generated = 1;
    
    entry->sym.up = labels->header_sym;
    labels->header_sym = &entry->sym;
    
    entry->link = labels->buckets[hash];
    labels->buckets[hash] = entry;
    
    // TODO: 通知后端，前端定义了一个符号，请求后端处理
    
    return &entry->sym;
}


/// 根据给定类型和值在constants符号表中查找常量，如果有需要，在该表中增加常量，函数返回一个指向常量符号的指针
/// @param ty 给定类型
/// @param val 给定值
Symbol constant(Type ty, Value val) {
    struct entry *entry;
    // 为什么是取unsigned_int_val?
    unsigned hash = val.unsigned_int_val & (HASH_SIZE - 1);
    for (entry = constants->buckets[hash]; entry; entry = entry->link) {
        if (equal_type(ty, entry->sym.type, 1)) {
            if (is_const_equal(ty, entry, val)) {
                return &entry->sym;
            }
        }
    }
    
    // install a new constant
    NEW0(entry, PERM);
    entry->sym.name = get_const_name(ty, val);
    entry->sym.scope = CONSTANTS;
    entry->sym.type = ty;
    entry->sym.storage_class = STATIC;
    entry->sym.extend_info.constant.val = val;
    
    entry->link = constants->buckets[hash];
    constants->buckets[hash] = entry;
    
    entry->sym.up = constants->header_sym;
    constants->header_sym = &entry->sym;
    if (ty->ex_info.sym && !ty->ex_info.sym->addressed) {
        // TODO:
    }
    
    entry->sym.defined = 1;
    
    return &entry->sym;
}

static bool is_const_equal(Type ty, struct entry *entry, Value val) {
    Value ty_val = entry->sym.extend_info.constant.val;
    switch (ty->op) {
        case CHAR:
            return ty_val.unsigned_char_val == val.unsigned_char_val;
        case SHORT:
            return ty_val.signed_short_val == val.signed_short_val;
        case INT:
            return ty_val.signed_int_val == val.signed_int_val;
        case UNSIGNED:
            return ty_val.unsigned_int_val == val.unsigned_int_val;
        case FLOAT:
            return ty_val.float_val == val.float_val;
        case DOUBLE:
            return ty_val.double_val == val.double_val;
        case ARRAY:
        case FUNCTION:
        case POINTER:
            return ty_val.pointer_val == val.pointer_val;
        default:
            assert(0);
    }
    
    return false;
}

char *get_const_name(Type ty, Value val) {
    // TODO:
    return NULL;
}
