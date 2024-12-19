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

// #define MAX_FILES 128
// PUBLIC int search_dir(char *path, char filenames[MAX_FILES][MAX_FILENAME_LEN])
// {
//     MESSAGE msg;
//     msg.type = SEARCH;
//     msg.PATHNAME = (void*)path;
//     msg.NAME_LEN = strlen(path);

//     send_recv(BOTH, TASK_FS, &msg);

//     int file_count = msg.CNT;
//     phys_copy((void*)filenames, (void*)msg.BUF, file_count * MAX_FILENAME_LEN);

//     return file_count;
// }
