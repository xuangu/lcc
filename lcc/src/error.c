//
//  error_1.c
//  lcc
//
//  Created by Gu,Xuan on 2020/7/27.
//  Copyright © 2020 Gu,Xuan. All rights reserved.
//

#include "c.h"

static int err_cnt = 0;
static int err_limit = 20;

void error(const char *fmt, ...) {
    va_list ap;

    if (err_cnt++ >= err_limit) {
        err_cnt = -1;
        // 这个递归设计很值得思考，是为了记录当err_cnt打到20时，也记录第20个错误的处理
        error("too many errors\n");
        exit(1);
    }

    va_start(ap, fmt);
    if (first_file != file && first_file && *first_file) {
        fprint(stderr, "s: ", first_file);
    }

    fprint(stderr, "%w: ", &src);
    vfprint(stderr, NULL, fmt, ap);
    va_end(ap);
}

