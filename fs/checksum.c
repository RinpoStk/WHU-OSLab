//
// Created by rinpostk on 24-12-22.
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
#include "proto.h"

#include "hd.h"


/*****************************************************************************
 *                                put_checksum
 *****************************************************************************/
PUBLIC void put_checksum() {
    char * pathname = fs_msg.PATHNAME;
    u8 * buf = fs_msg.BUF;

    int inode_nr = search_file(pathname);
    struct inode * dir_inode;
    char filename[MAX_PATH];
    if (strip_path(filename, pathname, &dir_inode) != 0)
       return;
    struct inode * pin = get_inode(dir_inode->i_dev, inode_nr);

    for (int i = 0; i < SYS_CHECKSUM_LEN; i++) {
       pin->i_checksum[i] = buf[i];
    }
    sync_inode(pin);
}

/*****************************************************************************
 *                                command_check
 *****************************************************************************/
PUBLIC void command_check() {
    int fd = fs_msg.FD;
    u8 * buf = fs_msg.BUF;
    fs_msg.FLAGS = 1;
    // for (int i = 0; i < SYS_CHECKSUM_LEN; i++) {
    //     if (pcaller->filp[fd]->fd_inode->i_checksum[i] != buf[i]) {
    //         fs_msg.FLAGS = 1;
    //         break;
    //     }
    // }
}
