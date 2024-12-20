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
#include "log.h"

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

	// return disklog(buf);
	return 0;
}

// #define MAX_BUF 512                     // read 1024 bytes

char * log_file_name = "syslog";        // no '/'

// char filelogbuf[MAX_BUF];               // in filelog.c


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

PUBLIC void custom_log(char * log) {
	MESSAGE msg;
	msg.type = PUT_CUSTOMLOG;
	msg.BUF  = log;

	send_recv(BOTH, TASK_SYS, &msg);
}
