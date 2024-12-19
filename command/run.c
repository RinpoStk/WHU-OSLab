#include"stdio.h"

int is_executable(const char *path) {
    struct stat s;
    int ret = stat(path, &s);
    if (ret != 0) {
        return 0; 
    }

    int fd = open(path, O_RDWR);
    if (fd == -1) {
        return 0; 
    }

    char magic[4];
    read(fd, magic, 4);
    close(fd);

    // 检查文件头是否为ELF文件头
    if (magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F') {
        return 1; 
    }

    return 0; 
}

int run_program(const char *path) {
    if (is_executable(path)) {
        exec(path);
        return 0;
    } else {
        printf("Error: %s is not an executable file.\n", path);
        return -1;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: run <file>\n");
        return 0;
    }

    return run_program(argv[1]);
}