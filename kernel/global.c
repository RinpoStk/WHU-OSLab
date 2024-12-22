/*************************************************************************//**
 *****************************************************************************
 * @file   global.c
 * @brief
 * @author Forrest Y. Yu
 * @date   2005
 *****************************************************************************
 *****************************************************************************/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "global.h"
#include "proto.h"


PUBLIC	struct proc proc_table[NR_TASKS + NR_PROCS];

/* 注意下面的 TASK 的顺序要与 const.h 中对应 */
PUBLIC	struct task	task_table[NR_TASKS] = {
    /* entry        stack size        task name */
    /* -----        ----------        --------- */
    {task_tty,      STACK_SIZE_TTY,   "TTY"       },
    {task_sys,      STACK_SIZE_SYS,   "SYS"       },
    {task_hd,       STACK_SIZE_HD,    "HD"        },
    {task_fs,       STACK_SIZE_FS,    "FS"        },
    {task_mm,       STACK_SIZE_MM,    "MM"        },
    {task_log,      STACK_SIZE_LOG,    "LOG"      } };

PUBLIC	struct task	user_proc_table[NR_NATIVE_PROCS] = {
    /* entry    stack size     proc name */
    /* -----    ----------     --------- */
    {Init,   STACK_SIZE_INIT,  "INIT" },
    {TestA,  STACK_SIZE_TESTA, "TestA"},
    {TestB,  STACK_SIZE_TESTB, "TestB"},
    {TestC,  STACK_SIZE_TESTC, "TestC"} };
/* PUBLIC	struct task	user_proc_table[NR_PROCS] = { */
/* 	{TestA, STACK_SIZE_TESTA, "TestA"}, */
/* 	{TestB, STACK_SIZE_TESTB, "TestB"}, */
/* 	{TestC, STACK_SIZE_TESTC, "TestC"}}; */

PUBLIC	char		task_stack[STACK_SIZE_TOTAL];

PUBLIC	TTY		tty_table[NR_CONSOLES];
PUBLIC	CONSOLE		console_table[NR_CONSOLES];

PUBLIC	irq_handler	irq_table[NR_IRQ];

PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = { sys_printx,
                               sys_sendrec };

/*process schedule*/
PUBLIC  int         cur_time_slice[NR_TASKS + NR_PROCS] = { 0 };
PUBLIC  PROC_QUEUE         MLFQ[NR_QUEUES];          //Multilevel Feedback Queue
PUBLIC  int idx = 0;

/* FS related below */
/*****************************************************************************/
/**
 * For dd_map[k],
 * `k' is the device nr.\ dd_map[k].driver_nr is the driver nr.
 *
 * Remeber to modify include/const.h if the order is changed.
 *****************************************************************************/
struct dev_drv_map dd_map[] = {
    /* driver nr.		major device nr.
       ----------		---------------- */
    {INVALID_DRIVER},	/**< 0 : Unused */
    {INVALID_DRIVER},	/**< 1 : Reserved for floppy driver */
    {INVALID_DRIVER},	/**< 2 : Reserved for cdrom driver */
    {TASK_HD},		/**< 3 : Hard disk */
    {TASK_TTY},		/**< 4 : TTY */
    {INVALID_DRIVER}	/**< 5 : Reserved for scsi disk driver */
};

/**
 * 6MB~7MB: buffer for FS
 */
PUBLIC	u8* fsbuf = (u8*)0x600000;
PUBLIC	const int	FSBUF_SIZE = 0x100000;


/**
 * 7MB~8MB: buffer for MM
 */
PUBLIC	u8* mmbuf = (u8*)0x700000;
PUBLIC	const int	MMBUF_SIZE = 0x100000;


PUBLIC int k_seed = 1;
PUBLIC int rand_times = 0;

/**
 * 8MB~10MB: buffer for log (debug)
 */
PUBLIC	char* logbuf = (char*)0x800000;
PUBLIC	const int	LOGBUF_SIZE = 0x100000;
PUBLIC	char* logdiskbuf = (char*)0x900000;
PUBLIC	const int	LOGDISKBUF_SIZE = 0x100000;


/* for log buf */
PUBLIC	char* custom_buf = (char*)0x1000000;
PUBLIC	const int	CUSTOMBUF_SIZE = 0x100000;

PUBLIC bool mm_buffull_flag = false;
PUBLIC bool fs_buffull_flag = false;
PUBLIC bool sys_buffull_flag = false;
PUBLIC bool hd_buffull_flag = false;
PUBLIC bool custom_buffull_flag = false;

// PUBLIC bool a = true;

PUBLIC int mm_log_bufpos = 0;
PUBLIC int fs_log_bufpos = 0;
PUBLIC int sys_log_bufpos = 0;
PUBLIC int hd_log_bufpos = 0;
PUBLIC int custom_log_bufpos = 0;
PUBLIC char mm_log_buf[MAX_LOG_BUF];
PUBLIC char fs_log_buf[MAX_LOG_BUF];
PUBLIC char sys_log_buf[MAX_LOG_BUF];
PUBLIC char hd_log_buf[MAX_LOG_BUF];
PUBLIC char custom_log_buf[MAX_LOG_BUF];

PUBLIC char* mm_log_file = "syslog_mm";
PUBLIC char* fs_log_file = "syslog_fs";
PUBLIC char* sys_log_file = "syslog_sys";
PUBLIC char* hd_log_file = "syslog_hd";
PUBLIC char* custom_log_file = "syslog_custom";


//for file encryption/decryption
PUBLIC unsigned int sysfile_cnt = 0xffffff00;
PUBLIC char file_crypt_key[MAX_FILE_CRYPT_KEYLEN] = { 0 };
PUBLIC unsigned int file_crypt_keylen = 0;

// MD5常量
PUBLIC const u32 S[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
					   5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
					   4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
					   6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

PUBLIC const u32 K[] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
					   0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
					   0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
					   0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
					   0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
					   0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
					   0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
					   0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
					   0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
					   0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
					   0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
					   0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
					   0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
					   0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
					   0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
					   0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

PUBLIC const u8 PADDING[] =	{0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// sys checksum
