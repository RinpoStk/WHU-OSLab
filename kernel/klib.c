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