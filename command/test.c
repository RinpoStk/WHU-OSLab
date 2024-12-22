#include"stdio.h"

int main(int argc, char * argv[]){

    int fd = open("a.txt", O_RDWR|O_CREAT);
    write(fd, "Hello, world!\n", 14);
    close(fd);
    fd = open("a.txt", O_RDWR);
    write(fd, "Hello, world1!\n", 14);
    close(fd);
}