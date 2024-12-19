#include"stdio.h"

int main(int argc, char *argv[]) {
    if(argc!=2){
        printf("Usage: cat <file>\n");
    }
    else{
        int fd=open(argv[1],O_RDWR);
        if(fd==-1){
            printf("cat: %s: No such file or directory\n",argv[1]);
        }
        else{
            char buf[1024];
            int n;
            while((n=read(fd,buf,1024))){
                printf("%s",buf);
                // write(1,buf,n);
            }
            printf("\n");
            close(fd);
        }
    }
}