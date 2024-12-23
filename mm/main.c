/*************************************************************************//**
 *****************************************************************************
 * @file   mm/main.c
 * @brief  Orange'S Memory Management.
 * @author Forrest Y. Yu
 * @date   Tue May  6 00:33:39 2008
 *****************************************************************************
 *****************************************************************************/

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

PUBLIC void do_fork_test();

PRIVATE void init_mm();

PRIVATE int do_get_proc_info(int src);


/*****************************************************************************
 *                                task_mm
 *****************************************************************************/
/**
 * <Ring 1> The main loop of TASK MM.
 * 
 *****************************************************************************/
PUBLIC void task_mm()
{
	init_mm();

	while (1) {
		send_recv(RECEIVE, ANY, &mm_msg);
		int src = mm_msg.source;
		int reply = 1;

		struct proc * p = proc_table;
		char * pname = p[src].name;

		int msgtype = mm_msg.type;

#ifdef ENABLE_FILE_LOG
		switch (msgtype) {
		case FORK:
			syslog_file(MMLOG, "[PORC %s, PID %d, FORK];\n", pname, src);
			break;
		case EXIT:
			syslog_file(MMLOG, "[PORC %s, PID %d, EXIT];\n", pname, src);
			break;
		case EXEC:
			syslog_file(MMLOG, "[PORC %s, PID %d, EXEC];\n", pname, src);
			break;
		case WAIT:
			syslog_file(MMLOG, "[PORC %s, PID %d, WAITIG];\n", pname, src);
			break;
		// case GET_PNAME:
			// syslog_file(MMLOG, "[PORC %s, PID %d, GETNAME];\n", pname, src);
			// break;
		// case FS_LOG:
		// 	char * porcname = p[mm_msg.PID].name;
		// 	printl("in mmmmmmmmm PID %d PNAME %s",mm_msg.PID ,porcname);
		// 	break;
		default:
			// syslog_file("[PORC %s, PID %d, DO UNKNOW];\n", pname, src);
			break;
		}
#endif

		// int flags = mm_msg.FLAGS;
		// if (flags == 10086)
		// 	return 0;

		switch (msgtype) {
		case FORK:
			mm_msg.RETVAL = do_fork();
			break;
		case EXIT:
			do_exit(mm_msg.STATUS);
			reply = 0;
			break;
		case EXEC:
			mm_msg.RETVAL = do_exec();
			break;
		case WAIT:
			do_wait();
			reply = 0;
			break;
		case GET_PROC_INFO:
			mm_msg.RETVAL = do_get_proc_info(src);
            break;
		case KILL:
		    mm_msg.RETVAL = do_kill(mm_msg.PID, mm_msg.STATUS);
			break;
		// case GET_PNAME:
		// 	mm_msg.RETVAL = do_getpname(src);
		// break;
		case FS_LOG:
			break;
		default:
			dump_msg("MM::unknown msg", &mm_msg);
			assert(0);
			break;
		}

		if (reply) {
			mm_msg.type = SYSCALL_RET;
			send_recv(SEND, src, &mm_msg);
		}
	}
}

/*****************************************************************************
 *                                init_mm
 *****************************************************************************/
/**
 * Do some initialization work.
 * 
 *****************************************************************************/
PRIVATE void init_mm()
{
	struct boot_params bp;
	get_boot_params(&bp);

	memory_size = bp.mem_size;

	/* print memory size */
	printl("{MM} memsize:%dMB\n", memory_size / (1024 * 1024));
}

/*****************************************************************************
 *                                alloc_mem
 *****************************************************************************/
/**
 * Allocate a memory block for a proc.
 * 
 * @param pid  Which proc the memory is for.
 * @param memsize  How many bytes is needed.
 * 
 * @return  The base of the memory just allocated.
 *****************************************************************************/
PUBLIC int alloc_mem(int pid, int memsize)
{
	assert(pid >= (NR_TASKS + NR_NATIVE_PROCS));
	if (memsize > PROC_IMAGE_SIZE_DEFAULT) {
		panic("unsupported memory request: %d. "
		      "(should be less than %d)",
		      memsize,
		      PROC_IMAGE_SIZE_DEFAULT);
	}

	int base = PROCS_BASE +
		(pid - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_IMAGE_SIZE_DEFAULT;

	if (base + memsize >= memory_size)
		panic("memory allocation failed. pid:%d", pid);

	return base;
}

/*****************************************************************************
 *                                free_mem
 *****************************************************************************/
/**
 * Free a memory block. Because a memory block is corresponding with a PID, so
 * we don't need to really `free' anything. In another word, a memory block is
 * dedicated to one and only one PID, no matter what proc actually uses this
 * PID.
 * 
 * @param pid  Whose memory is to be freed.
 * 
 * @return  Zero if success.
 *****************************************************************************/
PUBLIC int free_mem(int pid)
{
	return 0;
}

PRIVATE int do_get_proc_info(int src){
	struct proc *p_proc = proc_table;
	phys_copy(va2la(src,(void*)mm_msg.BUF), va2la(TASK_MM,(void*)p_proc), sizeof(struct proc) * (NR_TASKS+NR_PROCS));
	return 0;
}