//
// Created by rinpostk on 24-12-19.
//
#include "type.h"
#include "config.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PUBLIC void task_log(void) {
    init_sys_file();
    while (1) {
        // milli_delay(10000);
        if (mm_buffull_flag) {
        	filelog(MMLOG, mm_log_file);
        }

        if (fs_buffull_flag) {
        	filelog(FSLOG, fs_log_file);
        }

        if (sys_buffull_flag) {
        	// handle sys_log_bufpos overflow
        	filelog(SYSLOG, sys_log_file);
        }

        if (hd_buffull_flag) {
        	// handle hd_log_bufpos overflow
        	filelog(HDLOG, hd_log_file);
        }
    }
}


void init_sys_file(void) {
    char * log_list[4] = {mm_log_file, fs_log_file, sys_log_file, hd_log_file};
    for (int i = 0; i < 4; i++) {
        int fd = open(log_list[i], O_RDWR);
        if (fd == -1) {
            open(log_list[i], O_CREAT);
        } else {
            close(fd);
        }
    }
}
