#include "stdio.h"
#include "string.h"

char * syslog_mm = "syslog_mm";
char * syslog_fs = "syslog_fs";
char * syslog_sys = "syslog_sys";
char * syslog_hd = "syslog_hd";

int main(int argc, char * argv[])
{
	// printf("Log use help! log is set close by default\n");

    if(argc < 2){
        printf("Usage: log [--clean/ --crt/del(not recommend)] (it's not wise to enable a switch in console, to turn off the log func, pls check config)\n");
        return 1;
    }
    else{                           // at present can start log and see log
        if (!strcmp(argv[1], "se")){
            if(argc != 3){
                printf("Usage: log se [--mm/ --fs/ --sys/ --hd\n");
                return 1;
            }
            if (!strcmp(argv[2], "mm")){

                int fd=open(syslog_mm, O_RDWR);
                if(fd==-1){
                    printf("log: %s: this log file hasn't init yet\n",syslog_mm);
                }
                else{
                    char buf[2048];
                    int n;
                    while((n=read(fd,buf,2048))){
                        write(1,buf,n);
                    }
                    printf("\n");
                    close(fd);
                }
            }
            else if (!strcmp(argv[2], "fs")){
                // char * name = "syslog_fs";
                int fd=open(syslog_fs, O_RDWR);
                if(fd==-1){
                    printf("log: %s: this log file hasn't init yet\n",syslog_fs);
                }
                else{
                    char buf[2048];
                    int n;
                    while((n=read(fd,buf,2048))){
                        write(1,buf,n);
                    }
                    printf("\n");
                    close(fd);
                }
            }
            else if (!strcmp(argv[2], "sys")){
                // char * name = "syslog_sys";
                int fd=open(syslog_sys, O_RDWR);
                if(fd==-1){
                    printf("log: %s: this log file hasn't init yet\n",syslog_sys);
                }
                else{
                    char buf[2048];
                    int n;
                    while((n=read(fd,buf,2048))){
                        write(1,buf,n);
                    }
                    printf("\n");
                    close(fd);
                }
            }
            else if (!strcmp(argv[2], "hd")){
                // char * name = "syslog_hd";
                int fd=open(syslog_hd, O_RDWR);
                if(fd==-1){
                    printf("log: %s: this log file hasn't init yet\n",syslog_hd);
                }
                else{
                    char buf[2048];
                    int n;
                    while((n=read(fd,buf,2048))){
                        write(1,buf,n);
                    }
                    printf("\n");
                    close(fd);
                }
            }
        }
        else if (!strcmp(argv[1], "clean")){
            if(argc != 3){
                printf("Usage: log clean [--mm/ --fs/ --sys/ --hd\n");
                return 1;
            }
            if(!strcmp(argv[2], "mm")){
                unlink(syslog_mm);
                int fd = open(syslog_mm, O_CREAT);
                close(fd);
                return 0;
            }
            else if(!strcmp(argv[2], "fs")){
                unlink(syslog_fs);
                int fd = open(syslog_fs, O_CREAT);
                close(fd);
            }
            if(!strcmp(argv[2], "sys")){
                unlink(syslog_sys);
                int fd = open(syslog_sys, O_CREAT);
                close(fd);
                // return 0;
                // printf("fd %d\n", aaa);

            }
            if(!strcmp(argv[2], "hd")){
                unlink(syslog_hd);
                int fd = open(syslog_hd, O_CREAT);
                close(fd);
                // printf("fd %d\n", aaa);
                // return 0;
            }
        }
        else if (!strcmp(argv[1], "del")){
            if(argc != 3){
                printf("Usage: log del [--mm/ --fs/ --sys/ --hd\n");
                return 1;
            }

            if(!strcmp(argv[2], "mm")){
                unlink(syslog_mm);
                // int fd = open(syslog_mm, O_CREAT);
                // close(fd);
            }
            else if(!strcmp(argv[2], "fs")){
                int fd =  unlink(syslog_fs);
                printf("fd %d\n", fd);
                // int fd = open(syslog_fs, O_CREAT);
                // close(fd);
            }
            if(!strcmp(argv[2], "sys")){
                unlink(syslog_sys);
                // int fd = open(syslog_sys, O_CREAT);
                // close(fd);
            }
            if(!strcmp(argv[2], "hd")){
                unlink(syslog_hd);
                // int fd = open(syslog_hd, O_CREAT);
                // close(fd);
            }
        }
        else if (!strcmp(argv[1], "crt")){
            if(argc != 3){
                printf("Usage: log crt [--mm/ --fs/ --sys/ --hd\n");
                return 1;
            }

            if(!strcmp(argv[2], "mm")){
                // unlink(syslog_mm);
                int fd = open(syslog_mm, O_CREAT);
                close(fd);
            }
            else if(!strcmp(argv[2], "fs")){
                // unlink(syslog_fs);
                int fd = open(syslog_fs, O_CREAT);
                close(fd);
            }
            if(!strcmp(argv[2], "sys")){
                // unlink(syslog_sys);
                int fd = open(syslog_sys, O_CREAT);
                close(fd);
            }
            if(!strcmp(argv[2], "hd")){
                // unlink(syslog_hd);
                int fd = open(syslog_hd, O_CREAT);
                close(fd);
            }
        }
    }
    // log_command("abc");
	return 0;
}