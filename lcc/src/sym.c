//
//  symbol.c
//  lcc
//
//  Created by Gu,Xuan on 2020/7/27.
//  Copyright © 2020 Gu,Xuan. All rights reserved.
//

#include "c.h"

// 将某个作用域内的符号保存在一个哈希表中
struct table {
    // 作用域
    int scope_level;
    // 外层作用域
    Table outer;
    // entry实现了一个链式hash表，每个链表元素保存了一个symbol结构，和hash链中下一个入口的指针
    struct entry {
        struct symbol sym;
        struct entry *link;
    } *buckets[256];
    // 该域指向当前及其外层作用域中所有符号组成的链表的表头，该链表通过symbol结构的up域链接
    Symbol header_sym;
};

#define HASH_SIZE NELEMS(((Table)0)->buckets)

static struct table s_constants = { CONSTANTS };
static struct table s_externals = { GLOBAL };
static struct table s_identifiers = { GLOBAL };
static struct table s_types = { GLOBAL };

Table constants = &s_constants;
Table externals = &s_externals;
Table identifiers = &s_identifiers;
Table globals = &s_identifiers;
Table types = &s_types;
Table labels = NULL;

int level = GLOBAL;

Table alloc_table(Table table, int level) {
    Table tb;
    NEW0(tb, FUNC);
    
    tb->outer = table;
    tb->scope_level = level;
    
    if (table) {
        tb->header_sym = table->header_sym;
    }
    
    return table;
}

void foreach(Table table,
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
