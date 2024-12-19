/*************************************************************************//**
 *****************************************************************************
 * @file   exit.c
 * @brief  
 * @author Forrest Y. Yu
 * @date   Tue May  6 14:25:33 2008
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


/*****************************************************************************
 *                                exit
 *************************************************************************//**
 * Terminate the current process.
 * 
 * @param status  The value returned to the parent.
 *****************************************************************************/
PUBLIC void exit(int status)
{
	MESSAGE msg;
	msg.type	= EXIT;
	msg.STATUS	= status;

	send_recv(BOTH, TASK_MM, &msg);
	assert(msg.type == SYSCALL_RET);
}

// PUBLIC void kill(int pid,int status)
// {
// 	MESSAGE msg;
// 	msg.type	= KILL;
// 	msg.STATUS	= status;
// 	msg.PID = pid;

// 	send_recv(BOTH, TASK_MM, &msg);
// 	assert(msg.type == SYSCALL_RET);
// }

PUBLIC int kill(int pid) {
    MESSAGE msg;
    memset(&msg, 0, sizeof(MESSAGE));
    msg.type = KILL;
    msg.PID = pid;
    msg.STATUS = -1; 

    if (send_recv(BOTH, TASK_MM, &msg) != 0) {
        return -1; 
    }

    return msg.RETVAL;
}
