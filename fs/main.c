/*************************************************************************//**
 *****************************************************************************
 * @file   main.c
 * @brief
 * @author Forrest Y. Yu
 * @date   2007
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
#include "proto.h"

#include "hd.h"

PRIVATE void init_fs();
PRIVATE void mkfs();
PRIVATE void read_super_block(int dev);
PRIVATE int fs_fork();
PRIVATE int fs_exit();
PRIVATE int do_search_file();

/*****************************************************************************
 *                                task_fs
 *****************************************************************************/
/**
 * <Ring 1> The main loop of TASK FS.
 *
 *****************************************************************************/
PUBLIC void task_fs()
{
    printl("{FS} Task FS begins.\n");

    init_fs();

    while (1) {
        send_recv(RECEIVE, ANY, &fs_msg);

        int msgtype = fs_msg.type;
        int src = fs_msg.source;			// pid
        pcaller = &proc_table[src];
        char* pname = pcaller->name;		// proc name

#ifdef ENABLE_FILE_LOG
        char* msg_name[128];
        msg_name[OPEN] = "OPEN";
        msg_name[CLOSE] = "CLOSE";
        msg_name[READ] = "READ";
        msg_name[WRITE] = "WRITE";
        msg_name[LSEEK] = "LSEEK";
        msg_name[UNLINK] = "UNLINK";
        msg_name[FORK] = "FORK";
        msg_name[EXIT] = "EXIT";
        msg_name[STAT] = "STAT";
        msg_name[FS_CHECKSUM] = "FS_CHECKSUM";
        msg_name[FS_CHECK] = "FS_CHECK";
        msg_name[TRUNCATE] = "TRUNCATE";
        // printl("aaaain fs'open filelog\n");
        switch (msgtype) {
            case UNLINK:
                // dump_fd_graph("%s just finished. (pid:%d)",
                // 	      msg_name[msgtype], src);
                //panic("");
            case RESUME_PROC:
                break;
            case OPEN:
            // syslog_file(FSLOG, "[PORC %s, PID %d, %s];\n", pname, src, msg_name[OPEN]);
                break;
            case CLOSE:
            // syslog_file(FSLOG, "[PORC %s, PID %d, %s];\n", pname, src, msg_name[CLOSE]);
                break;
            case READ:
            // syslog_file(FSLOG, "[PORC %s, PID %d, %s];\n", pname, src, msg_name[READ]);
                break;
            case WRITE:
            // syslog_file(FSLOG, "[PORC %s, PID %d, %s];\n", pname, src, msg_name[WRITE]);
                break;
            case FORK:
            case EXIT:
            case LSEEK:
            case STAT:
            default:
                if ((char *)fs_msg.PATHNAME == '\x00' && strcmp(fs_msg.PATHNAME, ""))
                    syslog_file(FSLOG, "[PORC %s, PID %d, OPERATE %s %s];\n", pname, src, msg_name[msgtype], fs_msg.PATHNAME);
                else
                    syslog_file(FSLOG, "[PORC %s, PID %d, %s];\n", pname, src, msg_name[msgtype]);
                break;
        }
#endif

// printl("in fs'open filelog\n");
        switch (msgtype) {
            case OPEN:
                fs_msg.FD = do_open();
                break;
            case CLOSE:
                fs_msg.RETVAL = do_close();
                break;
            case READ:
                fs_msg.CNT = do_rdwt();
                char* buff = (char*)va2la(src, fs_msg.BUF);
                if (pcaller->filp[fs_msg.FD]->fd_inode->i_num > sysfile_cnt)
                {
                    file_crypt(buff, fs_msg.CNT, pcaller->filp[fs_msg.FD]->fd_pos - fs_msg.CNT);
                }
                break;
            case WRITE:
                buff = (char*)va2la(src, fs_msg.BUF);
                if (pcaller->filp[fs_msg.FD]->fd_inode->i_num > sysfile_cnt)
                {
                    file_crypt(buff, fs_msg.CNT, pcaller->filp[fs_msg.FD]->fd_pos);
                }
                fs_msg.CNT = do_rdwt();
                break;
            case UNLINK:
                fs_msg.RETVAL = do_unlink();
                break;
            // case SEARCH:
            //     fs_msg.RETVAL = do_search_file();
            //     break;
            case RESUME_PROC:
                src = fs_msg.PROC_NR;
                break;
            case FORK:
                fs_msg.RETVAL = fs_fork();
                break;
            case EXIT:
                fs_msg.RETVAL = fs_exit();
                break;
            case LSEEK:
                fs_msg.OFFSET = do_lseek();
                break;
            case STAT:
                fs_msg.RETVAL = do_stat();
                break;
            case TRUNCATE:
    		    fs_msg.RETVAL = do_truncate();
    		    break;
            case FS_CHECKSUM:
                fs_msg.BUF = (char*)va2la(src, fs_msg.BUF);
                fs_msg.PATHNAME = (char*)va2la(src, fs_msg.PATHNAME);
                put_checksum();
                break;
            case FS_CHECK:
                buff = (char*)va2la(src, fs_msg.BUF);
                command_check();
                break;
            default:
                dump_msg("FS::unknown message:", &fs_msg);
                assert(0);
                break;
        }

#ifdef ENABLE_DISK_LOG
        char* msg_name[128];
        msg_name[OPEN] = "OPEN";
        msg_name[CLOSE] = "CLOSE";
        msg_name[READ] = "READ";
        msg_name[WRITE] = "WRITE";
        msg_name[LSEEK] = "LSEEK";
        msg_name[UNLINK] = "UNLINK";
        msg_name[FORK] = "FORK";
        msg_name[EXIT] = "EXIT";
        msg_name[STAT] = "STAT";

        switch (msgtype) {
            case UNLINK:
                dump_fd_graph("%s just finished. (pid:%d)",
                          msg_name[msgtype], src);
                //panic("");
            case OPEN:

                syslog("[PORC %s, PID %d, %s]; ", name, src, msg_name[OPEN]);
            case CLOSE:
                syslog("[PORC %s, PID %d, %s]; ", name, src, msg_name[CLOSE]);
            case READ:
                syslog("[PORC %s, PID %d, %s]; ", name, src, msg_name[READ]);
            case WRITE:
                syslog("[PORC %s, PID %d, %s]; ", name, src, msg_name[WRITE]);
            case FORK:
                syslog("[PORC %s, PID %d, %s]; ", name, src, msg_name[FORK]);
            case EXIT:
                syslog("[PORC %s, PID %d, %s]; ", name, src, msg_name[EXIT]);
            case LSEEK:
                syslog("[PORC %s, PID %d, %s]; ", name, src, msg_name[EXIT]);
            case STAT:
                syslog("[PORC %s, PID %d, %s]; ", name, src, msg_name[EXIT]);
                break;
            case RESUME_PROC:
                break;
            default:
                assert(0);
        }
#endif

        /* reply */
        if (fs_msg.type != SUSPEND_PROC) {
            fs_msg.type = SYSCALL_RET;
            send_recv(SEND, src, &fs_msg);
        }
    }
}

/*****************************************************************************
 *                                init_fs
 *****************************************************************************/
/**
 * <Ring 1> Do some preparation.
 *
 *****************************************************************************/
PRIVATE void init_fs()
{
    int i;

    /* f_desc_table[] */
    for (i = 0; i < NR_FILE_DESC; i++)
        memset(&f_desc_table[i], 0, sizeof(struct file_desc));

    /* inode_table[] */
    for (i = 0; i < NR_INODE; i++)
        memset(&inode_table[i], 0, sizeof(struct inode));

    /* super_block[] */
    struct super_block* sb = super_block;
    for (; sb < &super_block[NR_SUPER_BLOCK]; sb++)
        sb->sb_dev = NO_DEV;

    /* open the device: hard disk */
    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    /* read the super block of ROOT DEVICE */
    RD_SECT(ROOT_DEV, 1);

    sb = (struct super_block*)fsbuf;
    if (sb->magic != MAGIC_V1) {
        printl("{FS} mkfs\n");
        mkfs(); /* make FS */
    }

    /* load super block of ROOT */
    read_super_block(ROOT_DEV);

    sb = get_super_block(ROOT_DEV);
    assert(sb->magic == MAGIC_V1);

    root_inode = get_inode(ROOT_DEV, ROOT_INODE);
}

/*****************************************************************************
 *                                mkfs
 *****************************************************************************/
/**
 * <Ring 1> Make a available Orange'S FS in the disk. It will
 *          - Write a super block to sector 1.
 *          - Create three special files: dev_tty0, dev_tty1, dev_tty2
 *          - Create a file cmd.tar
 *          - Create the inode map
 *          - Create the sector map
 *          - Create the inodes of the files
 *          - Create `/', the root directory
 *****************************************************************************/
PRIVATE void mkfs()
{
    MESSAGE driver_msg;
    int i, j;

    /************************/
    /*      super block     */
    /************************/
    /* get the geometry of ROOTDEV */
    struct part_info geo;
    driver_msg.type = DEV_IOCTL;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    driver_msg.REQUEST = DIOCTL_GET_GEO;
    driver_msg.BUF = &geo;
    driver_msg.PROC_NR = TASK_FS;
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    printl("{FS} dev size: 0x%x sectors\n", geo.size);

    int bits_per_sect = SECTOR_SIZE * 8; /* 8 bits per byte */
    /* generate a super block */
    struct super_block sb;
    sb.magic = MAGIC_V1; /* 0x111 */
    sb.nr_inodes = bits_per_sect;
    sb.nr_inode_sects = sb.nr_inodes * INODE_SIZE / SECTOR_SIZE;
    sb.nr_sects = geo.size; /* partition size in sector */
    sb.nr_imap_sects = 1;
    sb.nr_smap_sects = sb.nr_sects / bits_per_sect + 1;
    sb.n_1st_sect = 1 + 1 +   /* boot sector & super block */
        sb.nr_imap_sects + sb.nr_smap_sects + sb.nr_inode_sects;
    sb.root_inode = ROOT_INODE;
    sb.inode_size = INODE_SIZE;
    struct inode x;
    sb.inode_isize_off = (int)&x.i_size - (int)&x;
    sb.inode_start_off = (int)&x.i_start_sect - (int)&x;
    sb.dir_ent_size = DIR_ENTRY_SIZE;
    struct dir_entry de;
    sb.dir_ent_inode_off = (int)&de.inode_nr - (int)&de;
    sb.dir_ent_fname_off = (int)&de.name - (int)&de;

    memset(fsbuf, 0x90, SECTOR_SIZE);
    memcpy(fsbuf, &sb, SUPER_BLOCK_SIZE);

    /* write the super block */
    WR_SECT(ROOT_DEV, 1);

    printl("{FS} devbase:0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n"
           "        inodes:0x%x00, 1st_sector:0x%x00\n",
           geo.base * 2,
           (geo.base + 1) * 2,
           (geo.base + 1 + 1) * 2,
           (geo.base + 1 + 1 + sb.nr_imap_sects) * 2,
           (geo.base + 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects) * 2,
           (geo.base + sb.n_1st_sect) * 2);

    /************************/
    /*       inode map      */
    /************************/
    memset(fsbuf, 0, SECTOR_SIZE);
    for (i = 0; i < (NR_CONSOLES + 3); i++)
        fsbuf[0] |= 1 << i;

    assert(fsbuf[0] == 0x3F);/* 0011 1111 :
                  *   || ||||
                  *   || |||`--- bit 0 : reserved
                  *   || ||`---- bit 1 : the first inode,
                  *   || ||              which indicates `/'
                  *   || |`----- bit 2 : /dev_tty0
                  *   || `------ bit 3 : /dev_tty1
                  *   |`-------- bit 4 : /dev_tty2
                  *   `--------- bit 5 : /cmd.tar
                  */
    WR_SECT(ROOT_DEV, 2);

    /************************/
    /*      secter map      */
    /************************/
    memset(fsbuf, 0, SECTOR_SIZE);
    int nr_sects = NR_DEFAULT_FILE_SECTS + 1;
    /*             ~~~~~~~~~~~~~~~~~~~|~   |
     *                                |    `--- bit 0 is reserved
     *                                `-------- for `/'
     */
    for (i = 0; i < nr_sects / 8; i++)
        fsbuf[i] = 0xFF;

    for (j = 0; j < nr_sects % 8; j++)
        fsbuf[i] |= (1 << j);

    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects);

    /* zeromemory the rest sector-map */
    memset(fsbuf, 0, SECTOR_SIZE);
    for (i = 1; i < sb.nr_smap_sects; i++)
        WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + i);

    /* cmd.tar */
    /* make sure it'll not be overwritten by the disk log */
    assert(INSTALL_START_SECT + INSTALL_NR_SECTS <
           sb.nr_sects - NR_SECTS_FOR_LOG);
    int bit_offset = INSTALL_START_SECT -
        sb.n_1st_sect + 1; /* sect M <-> bit (M - sb.n_1stsect + 1) */
    int bit_off_in_sect = bit_offset % (SECTOR_SIZE * 8);
    int bit_left = INSTALL_NR_SECTS;
    int cur_sect = bit_offset / (SECTOR_SIZE * 8);
    RD_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);
    while (bit_left) {
        int byte_off = bit_off_in_sect / 8;
        /* this line is ineffecient in a loop, but I don't care */
        fsbuf[byte_off] |= 1 << (bit_off_in_sect % 8);
        bit_left--;
        bit_off_in_sect++;
        if (bit_off_in_sect == (SECTOR_SIZE * 8)) {
            WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);
            cur_sect++;
            RD_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);
            bit_off_in_sect = 0;
        }
    }
    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);

    /************************/
    /*       inodes         */
    /************************/
    /* inode of `/' */
    memset(fsbuf, 0, SECTOR_SIZE);
    struct inode* pi = (struct inode*)fsbuf;
    pi->i_mode = I_DIRECTORY;
    pi->i_size = DIR_ENTRY_SIZE * 5; /* 5 files:
                      * `.',
                      * `dev_tty0', `dev_tty1', `dev_tty2',
                      * `cmd.tar'
                      */
    pi->i_start_sect = sb.n_1st_sect;
    pi->i_nr_sects = NR_DEFAULT_FILE_SECTS;
    /* inode of `/dev_tty0~2' */
    for (i = 0; i < NR_CONSOLES; i++) {
        pi = (struct inode*)(fsbuf + (INODE_SIZE * (i + 1)));
        pi->i_mode = I_CHAR_SPECIAL;
        pi->i_size = 0;
        pi->i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
        pi->i_nr_sects = 0;
    }
    /* inode of `/cmd.tar' */
    pi = (struct inode*)(fsbuf + (INODE_SIZE * (NR_CONSOLES + 1)));
    pi->i_mode = I_REGULAR;
    pi->i_size = INSTALL_NR_SECTS * SECTOR_SIZE;
    pi->i_start_sect = INSTALL_START_SECT;
    pi->i_nr_sects = INSTALL_NR_SECTS;
    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + sb.nr_smap_sects);

    /************************/
    /*          `/'         */
    /************************/
    memset(fsbuf, 0, SECTOR_SIZE);
    struct dir_entry* pde = (struct dir_entry*)fsbuf;

    pde->inode_nr = 1;
    strcpy(pde->name, ".");

    /* dir entries of `/dev_tty0~2' */
    for (i = 0; i < NR_CONSOLES; i++) {
        pde++;
        pde->inode_nr = i + 2; /* dev_tty0's inode_nr is 2 */
        sprintf(pde->name, "dev_tty%d", i);
    }
    (++pde)->inode_nr = NR_CONSOLES + 2;
    strcpy(pde->name, "cmd.tar");
    WR_SECT(ROOT_DEV, sb.n_1st_sect);
}

/*****************************************************************************
 *                                rw_sector
 *****************************************************************************/
/**
 * <Ring 1> R/W a sector via messaging with the corresponding driver.
 *
 * @param io_type  DEV_READ or DEV_WRITE
 * @param dev      device nr
 * @param pos      Byte offset from/to where to r/w.
 * @param bytes    r/w count in bytes.
 * @param proc_nr  To whom the buffer belongs.
 * @param buf      r/w buffer.
 *
 * @return Zero if success.
 *****************************************************************************/
PUBLIC int rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_nr,
             void* buf)
{
    MESSAGE driver_msg;

    driver_msg.type = io_type;
    driver_msg.DEVICE = MINOR(dev);
    driver_msg.POSITION = pos;
    driver_msg.BUF = buf;
    driver_msg.CNT = bytes;
    driver_msg.PROC_NR = proc_nr;
    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);

    return 0;
}


/*****************************************************************************
 *                                read_super_block
 *****************************************************************************/
/**
 * <Ring 1> Read super block from the given device then write it into a free
 *          super_block[] slot.
 *
 * @param dev  From which device the super block comes.
 *****************************************************************************/
PRIVATE void read_super_block(int dev)
{
    int i;
    MESSAGE driver_msg;

    driver_msg.type = DEV_READ;
    driver_msg.DEVICE = MINOR(dev);
    driver_msg.POSITION = SECTOR_SIZE * 1;
    driver_msg.BUF = fsbuf;
    driver_msg.CNT = SECTOR_SIZE;
    driver_msg.PROC_NR = TASK_FS;
    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);

    /* find a free slot in super_block[] */
    for (i = 0; i < NR_SUPER_BLOCK; i++)
        if (super_block[i].sb_dev == NO_DEV)
            break;
    if (i == NR_SUPER_BLOCK)
        panic("super_block slots used up");

    assert(i == 0); /* currently we use only the 1st slot */

    struct super_block* psb = (struct super_block*)fsbuf;

    super_block[i] = *psb;
    super_block[i].sb_dev = dev;
}


/*****************************************************************************
 *                                get_super_block
 *****************************************************************************/
/**
 * <Ring 1> Get the super block from super_block[].
 *
 * @param dev Device nr.
 *
 * @return Super block ptr.
 *****************************************************************************/
PUBLIC struct super_block* get_super_block(int dev)
{
    struct super_block* sb = super_block;
    for (; sb < &super_block[NR_SUPER_BLOCK]; sb++)
        if (sb->sb_dev == dev)
            return sb;

    panic("super block of devie %d not found.\n", dev);

    return 0;
}


/*****************************************************************************
 *                                get_inode
 *****************************************************************************/
/**
 * <Ring 1> Get the inode ptr of given inode nr. A cache -- inode_table[] -- is
 * maintained to make things faster. If the inode requested is already there,
 * just return it. Otherwise the inode will be read from the disk.
 *
 * @param dev Device nr.
 * @param num I-node nr.
 *
 * @return The inode ptr requested.
 *****************************************************************************/
PUBLIC struct inode* get_inode(int dev, int num)
{
    if (num == 0)
        return 0;

    struct inode* p;
    struct inode* q = 0;
    for (p = &inode_table[0]; p < &inode_table[NR_INODE]; p++) {
        if (p->i_cnt) {	/* not a free slot */
            if ((p->i_dev == dev) && (p->i_num == num)) {
                /* this is the inode we want */
                p->i_cnt++;
                return p;
            }
        }
        else {		/* a free slot */
            if (!q) /* q hasn't been assigned yet */
                q = p; /* q <- the 1st free slot */
        }
    }

    if (!q)
        panic("the inode table is full");

    q->i_dev = dev;
    q->i_num = num;
    q->i_cnt = 1;

    struct super_block* sb = get_super_block(dev);
    int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects +
        ((num - 1) / (SECTOR_SIZE / INODE_SIZE));
    RD_SECT(dev, blk_nr);
    struct inode* pinode =
        (struct inode*)((u8*)fsbuf +
                ((num - 1) % (SECTOR_SIZE / INODE_SIZE))
                 * INODE_SIZE);
    q->i_mode = pinode->i_mode;
    q->i_size = pinode->i_size;
    q->i_start_sect = pinode->i_start_sect;
    q->i_nr_sects = pinode->i_nr_sects;
    return q;
}

/*****************************************************************************
 *                                put_inode
 *****************************************************************************/
/**
 * Decrease the reference nr of a slot in inode_table[]. When the nr reaches
 * zero, it means the inode is not used any more and can be overwritten by
 * a new inode.
 *
 * @param pinode I-node ptr.
 *****************************************************************************/
PUBLIC void put_inode(struct inode* pinode)
{
    assert(pinode->i_cnt > 0);
    pinode->i_cnt--;
}

/*****************************************************************************
 *                                sync_inode
 *****************************************************************************/
/**
 * <Ring 1> Write the inode back to the disk. Commonly invoked as soon as the
 *          inode is changed.
 *
 * @param p I-node ptr.
 *****************************************************************************/
PUBLIC void sync_inode(struct inode* p)
{
    struct inode* pinode;
    struct super_block* sb = get_super_block(p->i_dev);
    int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects +
        ((p->i_num - 1) / (SECTOR_SIZE / INODE_SIZE));
    RD_SECT(p->i_dev, blk_nr);
    pinode = (struct inode*)((u8*)fsbuf +
                 (((p->i_num - 1) % (SECTOR_SIZE / INODE_SIZE))
                     * INODE_SIZE));
    pinode->i_mode = p->i_mode;
    pinode->i_size = p->i_size;
    pinode->i_start_sect = p->i_start_sect;
    pinode->i_nr_sects = p->i_nr_sects;
    WR_SECT(p->i_dev, blk_nr);
}

/*****************************************************************************
 *                                fs_fork
 *****************************************************************************/
/**
 * Perform the aspects of fork() that relate to files.
 *
 * @return Zero if success, otherwise a negative integer.
 *****************************************************************************/
PRIVATE int fs_fork()
{
    int i;
    struct proc* child = &proc_table[fs_msg.PID];
    for (i = 0; i < NR_FILES; i++) {
        if (child->filp[i]) {
            child->filp[i]->fd_cnt++;
            child->filp[i]->fd_inode->i_cnt++;
        }
    }

    return 0;
}


/*****************************************************************************
 *                                fs_exit
 *****************************************************************************/
/**
 * Perform the aspects of exit() that relate to files.
 *
 * @return Zero if success.
 *****************************************************************************/
PRIVATE int fs_exit()
{
    int i;
    struct proc* p = &proc_table[fs_msg.PID];
    for (i = 0; i < NR_FILES; i++) {
        if (p->filp[i]) {
            /* release the inode */
            p->filp[i]->fd_inode->i_cnt--;
            /* release the file desc slot */
            if (--p->filp[i]->fd_cnt == 0)
                p->filp[i]->fd_inode = 0;
            p->filp[i] = 0;
        }
    }
    return 0;
}
// PRIVATE int do_search_file()
// {
//     char pathname[MAX_PATH];
//     char filenames[128][MAX_FILENAME_LEN];
//     int file_count = 0;

//     /* get parameters from the message */
//     int name_len = fs_msg.NAME_LEN;
//     int src = fs_msg.source;
//     assert(name_len < MAX_PATH);
//     phys_copy((void*)va2la(TASK_FS, pathname),
//               (void*)va2la(src, fs_msg.PATHNAME),
//               name_len);
//     pathname[name_len] = 0;

//     char filename[MAX_PATH];
//     struct inode *dir_inode;set_position

//     if (strip_path(filename, pathname, &dir_inode) != 0)
//         return 0;

//     int dir_blk0_nr = dir_inode->i_start_sect;
//     int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
//     int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;

//     struct dir_entry *pde;

//     for (int i = 0; i < nr_dir_blks; i++) {
//         RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
//         pde = (struct dir_entry *)fsbuf;
//         for (int j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++) {
//             if (pde->inode_nr != INVALID_INODE) {
//                 strncpy(filenames[file_count], pde->name, MAX_FILENAME_LEN);
//                 filenames[file_count][MAX_FILENAME_LEN - 1] = 0; // Ensure null-termination
//                 file_count++;
//                 if (file_count >= 128)
//                     break; // Reached maximum number of files
//             }
//             if (file_count > nr_dir_entries)
//                 break;
//         }
//         if (file_count > nr_dir_entries)
//             break;
//     }

//     /* send the result back to the caller */
//     fs_msg.CNT = file_count;
//     phys_copy((void*)va2la(src, fs_msg.BUF), (void*)filenames, file_count * MAX_FILENAME_LEN);

//     return 0;
// }
PUBLIC int do_truncate()
{
    int fd = fs_msg.FD;
    int length = fs_msg.CNT;

    if (length < 0) {
        return -1;
    }

    struct file_desc *fd_struct = pcaller->filp[fd];
    if (!fd_struct) {
        return -1;
    }
    struct inode *pin = fd_struct->fd_inode;
    if (!pin) {
        return -1;
    }

    if (length > pin->i_size) {
        return -1;
    }

    pin->i_size = length;
    fd_struct->fd_pos = length;

    sync_inode(pin);

    return 0;
}