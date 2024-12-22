//
// Created by rinpostk on 24-12-22.
//

#ifndef _ORANGES_SYSCALL_H
#define _ORANGES_SYSCALL_H

/**
 * MESSAGE mechanism is borrowed from MINIX
 */
struct mess1 {
    int m1i1;
    int m1i2;
    int m1i3;
    int m1i4;
};
struct mess2 {
    void* m2p1;
    void* m2p2;
    void* m2p3;
    void* m2p4;
};
struct mess3 {
    int	m3i1;
    int	m3i2;
    int	m3i3;
    int	m3i4;
    unsigned long long	m3l1;
    unsigned long long	m3l2;
    void*	m3p1;
    void*	m3p2;
};
typedef struct {
    int source;
    int type;
    union {
        struct mess1 m1;
        struct mess2 m2;
        struct mess3 m3;
    } u;
} MESSAGE;

PUBLIC void user_syscall(MESSAGE msg);

#endif //_ORANGES_SYSCALL_H
