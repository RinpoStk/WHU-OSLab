/*************************************************************************//**
 *****************************************************************************
 * @file   misc.c
 * @brief
 * @author Forrest Y. Yu
 * @date   2008
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
#include "keyboard.h"
#include "proto.h"
#include "syscall.h"

/*****************************************************************************
 *                                send_recv
 *****************************************************************************/
/**
 * <Ring 1~3> IPC syscall.
 *
 * It is an encapsulation of `sendrec',
 * invoking `sendrec' directly should be avoided
 *
 * @param function  SEND, RECEIVE or BOTH
 * @param src_dest  The caller's proc_nr
 * @param msg       Pointer to the MESSAGE struct
 * 
 * @return always 0.
 *****************************************************************************/
PUBLIC int send_recv(int function, int src_dest, MESSAGE* msg)
{
	int ret = 0;

	if (function == RECEIVE)
		memset(msg, 0, sizeof(MESSAGE));

	switch (function) {
	case BOTH:
		ret = sendrec(SEND, src_dest, msg);
		if (ret == 0)
			ret = sendrec(RECEIVE, src_dest, msg);
		break;
	case SEND:
	case RECEIVE:
		ret = sendrec(function, src_dest, msg);
		break;
	default:
		assert((function == BOTH) ||
		       (function == SEND) || (function == RECEIVE));
		break;
	}

	return ret;
}

/*****************************************************************************
 *                                memcmp
 *****************************************************************************/
/**
 * Compare memory areas.
 * 
 * @param s1  The 1st area.
 * @param s2  The 2nd area.
 * @param n   The first n bytes will be compared.
 * 
 * @return  an integer less than, equal to, or greater than zero if the first
 *          n bytes of s1 is found, respectively, to be less than, to match,
 *          or  be greater than the first n bytes of s2.
 *****************************************************************************/
PUBLIC int memcmp(const void * s1, const void *s2, int n)
{
	if ((s1 == 0) || (s2 == 0)) { /* for robustness */
		return (s1 - s2);
	}

	const char * p1 = (const char *)s1;
	const char * p2 = (const char *)s2;
	int i;
	for (i = 0; i < n; i++,p1++,p2++) {
		if (*p1 != *p2) {
			return (*p1 - *p2);
		}
	}
	return 0;
}

/*****************************************************************************
 *                                strcmp
 *****************************************************************************/
/**
 * Compare two strings.
 * 
 * @param s1  The 1st string.
 * @param s2  The 2nd string.
 * 
 * @return  an integer less than, equal to, or greater than zero if s1 (or the
 *          first n bytes thereof) is  found,  respectively,  to  be less than,
 *          to match, or be greater than s2.
 *****************************************************************************/
PUBLIC int strcmp(const char * s1, const char *s2)
{
	if ((s1 == 0) || (s2 == 0)) { /* for robustness */
		return (s1 - s2);
	}

	const char * p1 = s1;
	const char * p2 = s2;

	for (; *p1 && *p2; p1++,p2++) {
		if (*p1 != *p2) {
			break;
		}
	}

	return (*p1 - *p2);
}

PUBLIC char* strncpy(char* dst, const char* src, int n) {
    char* p = dst;
    while (n > 0 && *src != '\0') {
        *p++ = *src++;
        n--;
    }
    while (n > 0) {
        *p++ = '\0';
        n--;
    }
    return dst;
}

/*****************************************************************************
 *                                strcat
 *****************************************************************************/
/**
 * Concatenate two strings.
 * 
 * @param s1  The 1st string.
 * @param s2  The 2nd string.
 * 
 * @return  Ptr to the 1st string.
 *****************************************************************************/
PUBLIC char * strcat(char * s1, const char *s2)
{
	if ((s1 == 0) || (s2 == 0)) { /* for robustness */
		return 0;
	}

	char * p1 = s1;
	for (; *p1; p1++) {}

	const char * p2 = s2;
	for (; *p2; p1++,p2++) {
		*p1 = *p2;
	}
	*p1 = 0;

	return s1;
}

/*****************************************************************************
 *                                spin
 *****************************************************************************/
PUBLIC void spin(char * func_name)
{
	printl("\nspinning in %s ...\n", func_name);
	while (1) {}
}


/*****************************************************************************
 *                           assertion_failure
 *************************************************************************//**
 * Invoked by assert().
 *
 * @param exp       The failure expression itself.
 * @param file      __FILE__
 * @param base_file __BASE_FILE__
 * @param line      __LINE__
 *****************************************************************************/
PUBLIC void assertion_failure(char *exp, char *file, char *base_file, int line)
{
	printl("%c  assert(%s) failed: file: %s, base_file: %s, ln%d",
	       MAG_CH_ASSERT,
	       exp, file, base_file, line);

	/**
	 * If assertion fails in a TASK, the system will halt before
	 * printl() returns. If it happens in a USER PROC, printl() will
	 * return like a common routine and arrive here. 
	 * @see sys_printx()
	 * 
	 * We use a forever loop to prevent the proc from going on:
	 */
	spin("assertion_failure()");

	/* should never arrive here */
        __asm__ __volatile__("ud2");
}

// PUBLIC int get_proc_info(ProcInfo *proc_info_array, int max_procs) {
//     MESSAGE msg;
//     msg.type = GET_PROC_INFO;
//     msg.BUF = (ProcInfo*)proc_info_array;
//     msg.CNT = max_procs;

//     send_recv(BOTH, TASK_MM, &msg);
// 	ProcInfo *p=msg.BUF;
// 	dump_msg("msg:",&msg);
// 	for(int i=0;i<msg.RETVAL;i++){
// 		printf("%d  %s\n",i,p[i].name);
// 	}
//     return msg.RETVAL;
// }

PUBLIC int get_proc_info(struct proc *proc_info_array, int max_procs) {
	MESSAGE msg;
	msg.type = GET_PROC_INFO;
	msg.BUF = proc_info_array;
	msg.CNT = max_procs;

	send_recv(BOTH, TASK_MM, &msg);
	

	return msg.RETVAL;
}


PUBLIC int atoi(const char *str) {
	int result = 0;
	int sign = 1;
	if (*str == '-') {
		sign = -1;
		str++;
	}
	while (*str != '\0') {
		result = result * 10 + (*str - '0');
		str++;
	}
	return sign * result;
}

PUBLIC char *strchr(const char *str, int c) {
	// char *p = va2la(proc2pid(p_proc_ready), (void *)str);
	while (*str) {
		if (*str == (char)c) {
			return (char *)str;
		}
		str++;
	}
	return NULL;
}

PUBLIC char *strtok(char *str, const char *delim) {
	static char *last;
	if (str == NULL) {
		str = last;
	}
	if (str == NULL) {
		return NULL;
	}
	while (*str && strchr(delim, *str)) {
		str++;
	}
	if (*str == '\0') {
		last = NULL;
		return NULL;
	}
	char *token = str;
	while (*str && !strchr(delim, *str)) {
		str++;
	}
	if (*str) {
		*str = '\0';
		last = str + 1;
	} else {
		last = NULL;
	}
	return token;
}

PUBLIC void user_syscall(MESSAGE msg) {
	send_recv(BOTH, TASK_FS, &msg);
}
