/*************************************************************************//**
 *****************************************************************************
 * @file   syslog.c
 * @brief  
 * @author Forrest Y. Yu
 * @date   Thu Nov 20 17:02:42 2008
 *****************************************************************************
 *****************************************************************************/

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
#include "config.h"

/*****************************************************************************
 *                                syslog
 *****************************************************************************/
/**
 * Write log directly to the disk by sending message to FS.
 * 
 * @param fmt The format string.
 * 
 * @return How many chars have been printed.
 *****************************************************************************/
PUBLIC int syslog(const char *fmt, ...)
{
	int i;
	char buf[STR_DEFAULT_LEN];

	va_list arg = (va_list)((char*)(&fmt) + 4); /**
						     * 4: size of `fmt' in
						     *    the stack
						     */
	i = vsprintf(buf, fmt, arg);
	assert(strlen(buf) == i);

	return disklog(buf);
}

// #define MAX_BUF 512                     // read 1024 bytes

char * log_file_name = "syslog";        // no '/'
PRIVATE int read_register_log(char reg_addr);
PRIVATE u32 get_rtc_time_log(struct time *t);

// char filelogbuf[MAX_BUF];               // in filelog.c

char tmp1[STR_DEFAULT_LEN];


// char * mm_log_file = "syslog_mm";
// char * fs_log_file = "syslog_fs";
// char * sys_log_file = "syslog_sys";
// char * hd_log_file = "syslog_hd";

// new syslog beblow, by saving logs to files in the orangeOS
// so it needs an init first, but that maybe better put in the disklog.c
// for disklog.c is in fs while syslog is in lib
// as a inside file log, it should open the log file, and writes fmts into it
// but what about the pointer?
// whatever, the 1st to do is to choose a place for initialization.

// the two func should go to the same func in filelog.c, which is more effective and less work to do
// the true func is in the fs.h  --  fs/filelog.c

// If a buffer is to be used, then filelog should only write to the buffer.
// The definition and alter of buf should be completed in syslog_file.

PUBLIC int log_command(char * cmd){
	return 0;
//     // printf("crt_flag: %d\n", crt_flag);
//     if (!strcmp(cmd, "clean")){
//         unlink(log_file_name);
//         // printl("11111111\n");
//         int fd = open(log_file_name, O_CREAT);
//         assert(fd != 0);
//         close(fd);
//     }
//     else if (!strcmp(cmd, "crt")){                  // try not use cmd to do with log file
//         // if (crt_flag == 1){
//         //     printf("log file already exists!\n");
//         //     return 0;
//         // }
//         int fd = open(mm_log_file, O_CREAT);
//         close(fd);
//         fd = open(fs_log_file, O_CREAT);
//         close(fd);
//         fd = open(sys_log_file, O_CREAT);
//         close(fd);
//         fd = open(hd_log_file, O_CREAT);
//         close(fd);
//         // crt_flag = 1;
//     }
//     else if (!strcmp(cmd, "del")){
//         printf("It is not recommend to delet the log file\n");
//         unlink(log_file_name);
//         // crt_flag = 0;
//     }
//     return 0;
}

PUBLIC int syslog_file(int log_buf_flag, const char*fmt, ...)     // should be use to log, not for init
{
// —————此段保留，还是传入格式化字符串
	int i;
	char buf[STR_DEFAULT_LEN];
	va_list arg = (va_list)((char*)(&fmt) + 4); /**
						     * 4: size of `fmt' in
						     *    the stack
						     */
	i = vsprintf(buf, fmt, arg);
	assert(strlen(buf) == i);
// —————此段保留，还是传入格式化字符串
    // if (!strcmp(tmp1, buf)){
    //     // printl("same!!!!!!\n");
    //     return 0;           // if repeat
    // }

    memcpy(tmp1, buf, STR_DEFAULT_LEN);

    int pos;
    int log_len = strlen(buf);      // 添加的Log长度

    struct time t;
    MESSAGE msg;

    switch(log_buf_flag){
        case MMLOG:                 // mm_log_bufpos
            pos = mm_log_bufpos;
            if ((pos+24+log_len) > MAX_LOG_BUF){     // if over limit, stop push things into buf, wait for filelog
                mm_buffull_flag = true;
                break;
            }
            get_rtc_time_log(&t);
            sprintf((char*)mm_log_buf + pos, "<%d-%02d-%02d %02d:%02d:%02d>\n",	// 21 (for time)
                t.year,
                t.month,
                t.day,
                t.hour,
                t.minute,
                t.second
            );

            memset(mm_log_buf + pos + 22, ' ', 1);
            sprintf(mm_log_buf + pos + 23, buf);
            mm_log_buf[pos + 23 + log_len] = '\n';
            pos += 24 + log_len;
            mm_log_bufpos = pos;
            // if (mm_log_bufpos > 512)
            //     mm_log_bufpos = 0;
            break;
        case FSLOG:                                     // BUF len has some p
            pos = fs_log_bufpos;
            if ((pos+24+log_len) > MAX_LOG_BUF){     // if over limit, stop push things into buf, wait for filelog
                fs_buffull_flag = true;
                break;
            }
            get_rtc_time_log(&t);
            sprintf((char*)fs_log_buf + pos, "<%d-%02d-%02d %02d:%02d:%02d>\n",	// 21 (for time)
                t.year,
                t.month,
                t.day,
                t.hour,
                t.minute,
                t.second
            );

            memset(fs_log_buf + pos + 22, ' ', 1);
            sprintf(fs_log_buf + pos + 23, buf);
            fs_log_buf[pos + 23 + log_len] = '\n';
            pos += 24 + log_len;
            fs_log_bufpos = pos;
            break;
        case SYSLOG:            // 有可能加不了时间
            pos = sys_log_bufpos;
            if ((pos+24+log_len) > MAX_LOG_BUF){     // if over limit, stop push things into buf, wait for filelog
                sys_buffull_flag = true;
                break;
            }
            get_rtc_time_log(&t);
            sprintf((char*)sys_log_buf + pos, "<%d-%02d-%02d %02d:%02d:%02d>\n",	// 21 (for time)
                t.year,
                t.month,
                t.day,
                t.hour,
                t.minute,
                t.second
            );

            memset(sys_log_buf + pos + 22, ' ', 1);
            sprintf(sys_log_buf + pos + 23, buf);
            sys_log_buf[pos + 23 + log_len] = '\n';
            pos += 24 + log_len;
            sys_log_bufpos = pos;

            break;
        case HDLOG:
            pos = hd_log_bufpos;
            if ((pos+24+log_len) > MAX_LOG_BUF){     // if over limit, stop push things into buf, wait for filelog
                hd_buffull_flag = true;
                break;
            }
            get_rtc_time_log(&t);
            sprintf((char*)hd_log_buf + pos, "<%d-%02d-%02d %02d:%02d:%02d>\n",	// 21 (for time)
                t.year,
                t.month,
                t.day,
                t.hour,
                t.minute,
                t.second
            );

            memset(hd_log_buf + pos + 22, ' ', 1);
            sprintf(hd_log_buf + pos + 23, buf);
            hd_log_buf[pos + 23 + log_len] = '\n';
            pos += 24 + log_len;
            hd_log_bufpos = pos;

            break;
        default:
            // printl("default\n");
            assert(0);
            break;
    }
    // if (log_buf_flag == FSLOG){
    //     char tmp[fs_log_bufpos];
    //     memcpy(tmp, fs_log_buf, fs_log_bufpos);
    //     printl("mm_logbuf: %s\n", tmp);

    // }
        printl("sysbuf: %d\n", sys_log_bufpos);
        printl("flag %d\n", sys_buffull_flag);
	return 0;
}

// reme to alter the Makefile!!

// things should be done in this file:
/*
    1st initialization - by set a static var flag(or anything else)
    2nd write          - which means the log content should be sent in by syslog_file at first
                       - receive string <-- write(fd, str, len(str))
    3rd should return the postion in the file? the pointer should be set, but thats later things.
        preliminary return true.
*/

PUBLIC int filelog(int log_buf_flag, char * file_name){
#ifdef ENABLE_FILE_LOG

    char ch = 0x20;
    int fd = open(file_name, O_RDWR);
    lseek(fd,0,SEEK_END);
    switch(log_buf_flag) {
        case MMLOG:
            write(fd, mm_log_buf, mm_log_bufpos);
            memset(mm_log_buf, ch, mm_log_bufpos);
            mm_log_bufpos = 0;
            mm_buffull_flag = false;        // has been handled
            break;
        case FSLOG:
            write(fd, fs_log_buf, fs_log_bufpos);
            memset(fs_log_buf, ch, fs_log_bufpos);
            fs_log_bufpos = 0;
            fs_buffull_flag = false;
            break;
        case SYSLOG:
            write(fd, sys_log_buf, sys_log_bufpos);
            memset(sys_log_buf, ch, sys_log_bufpos);
            sys_log_bufpos = 0;
            sys_buffull_flag = false;
            break;
        case HDLOG:
            write(fd, hd_log_buf, hd_log_bufpos);
            memset(hd_log_buf, ch, hd_log_bufpos);
            hd_log_bufpos = 0;                      // write over, memset and back to zero
            hd_buffull_flag = false;
            break;
    }
    close(fd);

#endif

    return 0;
}


PRIVATE u32 get_rtc_time_log(struct time *t)
{
	t->year = read_register_log(YEAR);
	t->month = read_register_log(MONTH);
	t->day = read_register_log(DAY);
	t->hour = read_register_log(HOUR);
	t->minute = read_register_log(MINUTE);
	t->second = read_register_log(SECOND);

	if ((read_register_log(CLK_STATUS) & 0x04) == 0) {
		/* Convert BCD to binary (default RTC mode) */
		t->year = BCD_TO_DEC(t->year);
		t->month = BCD_TO_DEC(t->month);
		t->day = BCD_TO_DEC(t->day);
		t->hour = BCD_TO_DEC(t->hour);
		t->minute = BCD_TO_DEC(t->minute);
		t->second = BCD_TO_DEC(t->second);
	}

	t->year += 2000;

	return 0;
}

PRIVATE int read_register_log(char reg_addr)
{
	out_byte(CLK_ELE, reg_addr);
	return in_byte(CLK_IO);
}
