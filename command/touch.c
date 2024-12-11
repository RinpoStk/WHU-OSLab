#include"stdio.h"

int main(int argc, char * argv[]){
    if(argc != 2)
        printf("Usage: touch <file>\n");
    else{
        int fd = open(argv[1], O_CREAT);
        if(fd == -1)
            printf("touch: %s: File create failed\n", argv[1]);
        else
            close(fd);
    }
    return 0;
}