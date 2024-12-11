#include"stdio.h"
#include"string.h"
#include"../include/sys/protect.h"
#include"../include/sys/const.h"
#include"../include/sys/proc.h"

int main(int argc, char * argv[]){
    if(argc != 2)
        printf("Usage: kill <pid>\n");
    else{
        int pid = atoi(argv[1]);
        if(pid < NR_TASKS+NR_NATIVE_PROCS+2)
            printf("kill: %d: cannot kill tasks and native processes!\n", pid);
        else if(pid == getpid())
            printf("kill: %d: cannot kill current process!\n", pid);
        else{
            if(kill(pid) == -1)
                printf("kill: %d: No such process\n", pid);
            else
                printf("kill: %d: Process killed\n", pid);
        }
    }
    return 0;
}