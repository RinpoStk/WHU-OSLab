#include"type.h"
#include"stdio.h"
#include"../include/sys/protect.h"
#include"../include/sys/const.h"
#include"../include/sys/proc.h"

int main(int argc,char *argv[]){
    struct proc proc_info_array[NR_TASKS+NR_PROCS];
    get_proc_info(proc_info_array, NR_TASKS+NR_PROCS);
    int proc_count = 0;
    printf("PID\tNAME\n");
    for (int i = 0; i < NR_TASKS+NR_PROCS; i++) {
        if (proc_info_array[i].p_flags != FREE_SLOT) {
            proc_count++;
            // printf("%d",proc_info_array[i].p_flags);
            printf("%d\t%s\n", i, proc_info_array[i].name);
        }
    }
    printf("Total %d processes.\n", proc_count);
    return 0;
}