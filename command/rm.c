#include"stdio.h"

int main(int argc, char * argv[]){
    if(argc != 2)
        printf("Usage: rm <file>\n");
    else{
        if(unlink(argv[1]) == -1)
            printf("rm: %s: No such file or directory\n", argv[1]);
        else
            printf("rm: %s: File removed\n", argv[1]);
    }
    return 0;
}