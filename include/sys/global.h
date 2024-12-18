
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int	ticks;

EXTERN	int	disp_pos;

EXTERN	u8			gdt_ptr[6];	/* 0~15:Limit  16~47:Base */
EXTERN	struct descriptor	gdt[GDT_SIZE];
EXTERN	u8			idt_ptr[6];	/* 0~15:Limit  16~47:Base */
EXTERN	struct gate		idt[IDT_SIZE];

EXTERN	u32	k_reenter;
EXTERN	int	current_console;

EXTERN	int	key_pressed; /**
			      * used for clock_handler
			      * to wake up TASK_TTY when
			      * a key is pressed
			      */

EXTERN	struct tss	tss;
EXTERN	struct proc*	p_proc_ready;

extern	char		task_stack[];
extern	struct proc	proc_table[];
extern  struct task	task_table[];
extern  struct task	user_proc_table[];
extern	irq_handler	irq_table[];
extern	TTY		tty_table[];
extern  CONSOLE		console_table[];

/*process schedule*/
extern  PROC_QUEUE     MLFQ[NR_QUEUES];
extern  int         cur_time_slice[NR_TASKS + NR_PROCS];
extern  int idx;

/* MM */
EXTERN	MESSAGE			mm_msg;
extern	u8 *			mmbuf;
extern	const int		MMBUF_SIZE;
EXTERN	int			memory_size;

/* FS */
EXTERN	struct file_desc	f_desc_table[NR_FILE_DESC];
EXTERN	struct inode		inode_table[NR_INODE];
EXTERN	struct super_block	super_block[NR_SUPER_BLOCK];
extern	u8 *			fsbuf;
extern	const int		FSBUF_SIZE;
EXTERN	MESSAGE			fs_msg;
EXTERN	struct proc *		pcaller;
EXTERN	struct inode *		root_inode;
extern	struct dev_drv_map	dd_map[];

// RANDOM
EXTERN	int k_seed;
EXTERN	int rand_times;

/* for test only */
extern	char *			logbuf;
extern	const int		LOGBUF_SIZE;
extern	char *			logdiskbuf;
extern	const int		LOGDISKBUF_SIZE;

/*for memory management*/
EXTERN  int     PageDirBase;
EXTERN  int     PageTblBase;
EXTERN  int     dwPDENum;
EXTERN  u8      szPageFreeErr[20];
EXTERN  u8      szPageAllocErr[20];

// log

EXTERN	int				log_gate;

/* for log buf */
// extern bool a;

extern bool mm_buffull_flag;
extern bool fs_buffull_flag;
extern bool sys_buffull_flag;
extern bool hd_buffull_flag;

extern int mm_log_bufpos;
extern int fs_log_bufpos;
extern int sys_log_bufpos;
extern int hd_log_bufpos;
extern char mm_log_buf[];
extern char fs_log_buf[];
extern char sys_log_buf[];
extern char hd_log_buf[];

extern char * mm_log_file;
extern char * fs_log_file;
extern char * sys_log_file;
extern char * hd_log_file;