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
#include "time.h"

PUBLIC int get_time(void) {
    MESSAGE msg;
    memset(&msg, 0, sizeof(MESSAGE));
    msg.type = GET_TICKS;
    send_recv(BOTH, TASK_SYS, &msg);
    return msg.RETVAL;
}
