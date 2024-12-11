#include "stdio.h"
#include "../include/sys/const.h"
#include "type.h"

int main(int argc, char *argv[]) {
    int print_size = 0;
    int print_inode = 0;

    // 检查是否有 -s 和 -i 参数
    for (int arg = 1; arg < argc; arg++) {
        if (strcmp(argv[arg], "-s") == 0) {
            print_size = 1;
        } else if (strcmp(argv[arg], "-i") == 0) {
            print_inode = 1;
        }
    }

    int fd = open(".", O_RDWR);
    if (fd == -1) {
        printf("Failed to open root directory.\n");
        return 0;
    }

    struct dir_entry {
        int inode_nr;
        char name[12];
    } dir_entries[64];

    int n = read(fd, dir_entries, sizeof(dir_entries));
    if (n == -1) {
        printf("Failed to read root directory.\n");
        close(fd);
        return 0;
    }

    // 打印标题
    if (print_inode && print_size) {
        printf("Name\t\tSize(byte)\tInode\n");
    } else if (print_inode) {
        printf("Name\t\tInode\n");
    } else if (print_size) {
        printf("Name\t\tSize(byte)\n");
    } else {
        printf("Name\n");
    }

    // 打印目录内容
    for (int i = 0; i < n / sizeof(struct dir_entry); i++) {
        if (dir_entries[i].inode_nr != 0) {
            struct stat file_stat;
            if (stat(dir_entries[i].name, &file_stat) == 0) {
                if (print_inode && print_size) {
                    printf("%s\t\t%d\t\t%d\n", dir_entries[i].name, file_stat.st_size, file_stat.st_ino);
                } else if (print_inode) {
                    printf("%s\t\t%d\n", dir_entries[i].name, file_stat.st_ino);
                } else if (print_size) {
                    printf("%s\t\t%d\n", dir_entries[i].name, file_stat.st_size);
                } else {
                    printf("%s\n", dir_entries[i].name);
                }
            } else {
                printf("%s\t\tError\n", dir_entries[i].name);
            }
        }
    }

    close(fd);
    return 0;
}