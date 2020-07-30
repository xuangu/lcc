//
//  types.c
//  lcc
//
//  Created by guxuan on 2020/7/29.
//  Copyright © 2020 Gu,Xuan. All rights reserved.
//

#include "c.h"

void rm_sym_from_types_table(int level) {
    
}

bool is_qualified(Type ty) {
    return ty->op == CONST || ty->op == VOLATILE;
}

Type unqualify(Type ty) {
    return is_qualified(ty) ? ty->type : ty;
}

bool equal_type(Type ty1, Type ty2, int arr_empty_ret_val) {
    return false;
}
