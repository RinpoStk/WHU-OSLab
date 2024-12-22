
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            type.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef	_ORANGES_TYPE_H_
#define	_ORANGES_TYPE_H_

/* routine types */
#define	PUBLIC		/* PUBLIC is the opposite of PRIVATE */
#define	PRIVATE	static	/* PRIVATE x limits the scope of x */

#define bool 	_Bool
#define true 	1
#define false 	0

typedef	unsigned long long	u64;
typedef	unsigned int		u32;
typedef	unsigned short		u16;
typedef	unsigned char		u8;

typedef	char *			va_list;

typedef	void	(*int_handler)	();
typedef	void	(*task_f)	();
typedef	void	(*irq_handler)	(int irq);

typedef void*	system_call;

#include "syscall.h"

/* i have no idea of where to put this struct, so i put it here */
struct boot_params {
	int		mem_size;	/* memory size */
	unsigned char *	kernel_file;	/* addr of kernel file */
};

typedef struct procinfo{
    int pid;
    char name[16];
} ProcInfo;

#endif /* _ORANGES_TYPE_H_ */
