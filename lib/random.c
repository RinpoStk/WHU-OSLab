//
// Created by rinpostk on 24-12-18.
//

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "random.h"

PRIVATE int __seed = 1;

PUBLIC void srand(int seed) {
    if (seed == 0) {
        __seed = 1;
    }
    __seed = seed;
}

PUBLIC int rand(void) {
    u64 tmp = LCG_A * __seed + LCG_C;
    __seed = tmp % LCG_M;
    return __seed;
}
