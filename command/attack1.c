#include "stdio.h"
int i;
int* addr;

void test() {
    printf("Wuhan University School of Cyber Science and Engineering");
}

void testat() {
    char buff[72] = {0};
    for (i = 0; i < 72; i++) {
        buff[i] = 0;
    }

    for (; i < 72; i++) {
        buff[i] = 0;
    }
    addr=&buff[72];
    for (i = 0; i < 4; i++) {
        addr[i] = 0x1000;
    }
    // printf("asd");
}

void main(int argc, char* argv[]) {
    // printf("asd");
    testat();
}
