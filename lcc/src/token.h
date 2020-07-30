//
//  token.h
//  lcc
//
//  Created by Gu,Xuan on 2020/7/30.
//  Copyright © 2020 Gu,Xuan. All rights reserved.
//

#ifndef token_h
#define token_h

enum token_symbol {
    FLOAT = 1,
    DOUBLE = 2,
    CHAR = 3,
    SHORT = 4,
    INT,
    UNSIGNED,
    POINTER,
    VOID,
    STRUCT,
    UNION,
    FUNCTION,
    ARRAY,
    ENUM,
    LONG,
    CONST,
    VOLATILE,
    STATIC = 81,
};

#endif /* token_h */
