/*************************************************************************//**
 *****************************************************************************
 * @file   klib.c
 * @brief
 * @author Forrest Y. Yu
 * @date   2005
 *****************************************************************************
 *****************************************************************************/

#include "type.h"
#include "config.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "krandom.h"
#include "md5.h"

#include "elf.h"


/*****************************************************************************
 *                                get_boot_params
 *****************************************************************************/
/**
 * <Ring 0~1> The boot parameters have been saved by LOADER.
 *            We just read them out.
 *
 * @param pbp  Ptr to the boot params structure
 *****************************************************************************/
PUBLIC void get_boot_params(struct boot_params* pbp)
{
    /**
     * Boot params should have been saved at BOOT_PARAM_ADDR.
     * @see include/load.inc boot/loader.asm boot/hdloader.asm
     */
    int* p = (int*)BOOT_PARAM_ADDR;
    assert(p[BI_MAG] == BOOT_PARAM_MAGIC);

    pbp->mem_size = p[BI_MEM_SIZE];
    pbp->kernel_file = (unsigned char*)(p[BI_KERNEL_FILE]);

    /**
     * the kernel file should be a ELF executable,
     * check it's magic number
     */
    assert(memcmp(pbp->kernel_file, ELFMAG, SELFMAG) == 0);
}


/*****************************************************************************
 *                                get_kernel_map
 *****************************************************************************/
/**
 * <Ring 0~1> Parse the kernel file, get the memory range of the kernel image.
 *
 * - The meaning of `base':	base => first_valid_byte
 * - The meaning of `limit':	base + limit => last_valid_byte
 *
 * @param b   Memory base of kernel.
 * @param l   Memory limit of kernel.
 *****************************************************************************/
PUBLIC int get_kernel_map(unsigned int* b, unsigned int* l)
{
    struct boot_params bp;
    get_boot_params(&bp);

    Elf32_Ehdr* elf_header = (Elf32_Ehdr*)(bp.kernel_file);

    /* the kernel file should be in ELF format */
    if (memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0)
        return -1;

    *b = ~0;
    unsigned int t = 0;
    int i;
    for (i = 0; i < elf_header->e_shnum; i++) {
        Elf32_Shdr* section_header =
            (Elf32_Shdr*)(bp.kernel_file +
                      elf_header->e_shoff +
                      i * elf_header->e_shentsize);
        if (section_header->sh_flags & SHF_ALLOC) {
            int bottom = section_header->sh_addr;
            int top = section_header->sh_addr +
                section_header->sh_size;

            if (*b > bottom)
                *b = bottom;
            if (t < top)
                t = top;
        }
    }
    assert(*b < t);
    *l = t - *b - 1;

    return 0;
}

/*======================================================================*
                               itoa
 *======================================================================*/
PUBLIC char* itoa(char* str, int num)/* 数字前面的 0 不被显示出来, 比如 0000B800 被显示成 B800 */
{
    char* p = str;
    char	ch;
    int	i;
    int	flag = 0;

    *p++ = '0';
    *p++ = 'x';

    if (num == 0){
        *p++ = '0';
    }
    else{
        for (i = 28;i >= 0;i -= 4){
            ch = (num >> i) & 0xF;
            if (flag || (ch > 0)){
                flag = 1;
                ch += '0';
                if (ch > '9'){
                    ch += 7;
                }
                *p++ = ch;
            }
        }
    }

    *p = 0;

    return str;
}


/*======================================================================*
                               disp_int
 *======================================================================*/
PUBLIC void disp_int(int input)
{
    char output[16];
    itoa(output, input);
    disp_str(output);
}

/*======================================================================*
                               delay
 *======================================================================*/
PUBLIC void delay(int time)
{
    int i, j, k;
    for (k = 0;k < time;k++){
        /*for(i=0;i<10000;i++){	for Virtual PC	*/
        for (i = 0;i < 10;i++){/*	for Bochs	*/
            for (j = 0;j < 10000;j++){}
        }
    }
}

/*======================================================================*
                               random
 *======================================================================*/
PUBLIC int random(void) {
    if (rand_times >= 624 || rand_times == 0) {
        rand_times = 0;
        k_seed = proc_table[1].ticks;
    }
    rand_times += 1;
    u64 tmp = _RANDOM_A * k_seed + _RANDOM_C;
    k_seed = tmp % _RANDOM_M;
    return k_seed;
}

/*======================================================================*
                        file encrypt/decrypt
 *======================================================================*/

void rc4(char* data, char* key, int datalen, int keylen, int pos)
{
    unsigned char sbox[256] = { 0 }, tmp;
    int i = 0, j = 0, k = 0;

    for (i = 0;i < 256;i++)
        sbox[i] = i;
    for (i = 0;i < 256;i++)
    {
        j = (j + sbox[i] + key[i % keylen]) % 256;
        tmp = sbox[j];
        sbox[j] = sbox[i];
        sbox[i] = tmp;
    }
    i = j = 0;
    for (k = 0;k < datalen + pos;k++)
    {
        i = (i + 1) % 256;
        j = (j + sbox[i]) % 256;
        tmp = sbox[j];
        sbox[j] = sbox[i];
        sbox[i] = tmp;
        if (k >= pos)
            data[k] ^= sbox[(sbox[i] + sbox[j]) % 256];
    }
}

PUBLIC int check_passwd(char* passwd, unsigned int passwdlen)
{
    char enc[] = { 0x02,0x4b,0xa1,0x72,0xec,0xc6,0xe8,0xd3,0xca,0xb0,0x1a,0x62,0xe8,0x60,0x09,0x49,0x02,0xb4 }, dec[] = "Siestazzz_Loves_OS";
    u8 enclen = 18;
    rc4(enc, passwd, enclen, passwdlen, 0);
    return !memcmp(enc, dec, enclen);
}

PUBLIC void file_crypt(char* data, unsigned int datalen, unsigned int pos)
{
    rc4(data, file_crypt_key, datalen, file_crypt_keylen, pos);
}

/*======================================================================*
							   md5
 *======================================================================*/
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


PUBLIC void checksum_md5(void *input, u32 length, char *result){
    MD5Context ctx;
    
    // init md5 ctx
    ctx.size = (u64)0;
    ctx.buffer[0] = (u32)0x67452301;
    ctx.buffer[1] = (u32)0xefcdab89;
    ctx.buffer[2] = (u32)0x98badcfe;
    ctx.buffer[3] = (u32)0x10325476;

    md5_update(&ctx, input, length);
    md5_finalize(&ctx);

    for (int i = 0; i < MD5_DIGEST_SIZE; ++i) {
        result[i] = ctx.digest[i];
    }
}

PUBLIC void checksum_md5_file(int fd, char *result) {
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

// sec
PUBLIC int put_canary() {
    int canary = random();
    asm("movl %0, %%gs:0x28" : "=r"(canary));

}

PUBLIC void canary_check(int value){
    int canary;
    asm("movl %%gs:0x28, %0" : "=r"(canary));
    // assert(canary==value);
    if (value != canary) {
        panic("stack smashing detected!!\n");
    }
}
