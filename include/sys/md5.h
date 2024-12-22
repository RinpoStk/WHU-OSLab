//
// Created by rinpostk on 24-12-21.
//

#ifndef _ORANGES_MD5_H
#define _ORANGES_MD5_H

#define MD5_BLOCK_SIZE 64      // MD5处理块的大小
#define MD5_DIGEST_SIZE 16

#define MD5_F(X, Y, Z) ((X & Y) | (~X & Z))
#define MD5_G(X, Y, Z) ((X & Z) | (Y & ~Z))
#define MD5_H(X, Y, Z) (X ^ Y ^ Z)
#define MD5_I(X, Y, Z) (Y ^ (X | ~Z))
#define ROTATE_LEFT(X, N) ((X << N) | (X >> (32 - N)))

extern const u32 S[];
extern const u32 K[];
extern const u8 PADDING[];

typedef struct{
    u64 size;        // 输入长度
    u32 buffer[4];   // Current accumulation of hash
    u8 input[64];    // Input to be used in the next step
    u8 digest[16];   // 结果
}MD5Context;

#endif //_ORANGES_MD5_H
