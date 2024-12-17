#include"stdio.h"

int main(int argc, char * argv[]){
    for (int i = 0; i < argc; i++) {
        printl("%s\n ", argv[i]);
    }
    if(argc != 2)
        printf("Usage: rm <file>\n");
    else{
        if(unlink(argv[1]) == -1)
            printl("No such file or directory\n");
        else
            printl("File removed\n");
    }
    return 0;
}