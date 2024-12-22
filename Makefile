#########################
# Makefile for Orange'S #
#########################

# Entry point of Orange'S
# It must have the same value with 'KernelEntryPointPhyAddr' in load.inc!
ENTRYPOINT	= 0x1000

FD		= a.img
HD		= 80m.img

# Programs, flags, etc.
AR		= ar
ASM		= nasm
DASM	= objdump
CC		= gcc
LD		= ld

ASMBFLAGS	= -I boot/include/
ASMKFLAGS	= -g -I include/ -I include/sys/ -f elf
CFLAGS		= -m32 -g -I include/ -I include/sys/ -c -fno-builtin -Wall -fno-stack-protector
LDFLAGS		= -m elf_i386 -Ttext $(ENTRYPOINT) -Map krnl.map
DASMFLAGS	= -D
ARFLAGS		= rcs

# This Program
ORANGESBOOT	= boot/boot.bin boot/hdboot.bin boot/loader.bin boot/hdloader.bin
ORANGESKERNEL	= kernel.bin
SYMKERNEL = kernel.dbg

LIB	= lib/orangescrt.a
LIBSRC 	= $(wildcard lib/*.c) $(wildcard lib/*.asm)
LIBOBJS = $(LIBSRC:.c=.o) $(LIBSRC:.asm=.o)

SRCDIRS = kernel lib fs mm
SRCASM 	= $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.asm))
SRCC 	= $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))

OBJS = $(SRCASM:.asm=.o) $(SRCC:.c=.o)

DASMOUTPUT	= kernel.bin.asm

# command compile
CASMFLAGS 	= -I include/ -f elf
CCFLAGS 	= -m32 -fno-pie -I include/ -c -fno-builtin -fno-stack-protector -Wall
CLDFLAGS 	= -Ttext $(ENTRYPOINT) -m elf_i386

# command src
CSRCDIR = command
CSRC 	= $(wildcard $(CSRCDIR)/*.c)
CBIN 	= $(CSRC:.c=)
COBJS	= $(CSRC:.c=.o)
CSTART	= $(CSRCDIR)/start.asm
CSTARTOBJ= $(CSRCDIR)/start.o

# All Phony Targets
.PHONY : everything final image clean realclean disasm all buildimg ccommand

# Default starting position
nop :
	@echo "why not \`make image' huh? :)"

everything : $(ORANGESBOOT) $(ORANGESKERNEL) $(CSTARTOBJ) $(CBIN)

split : everything
	objcopy --only-keep-debug $(ORANGESKERNEL) $(SYMKERNEL)
	strip --strip-debug $(ORANGESKERNEL)

all : realclean split ccommand

image : realclean split ccommand buildimg clean

clean :
	rm -f $(OBJS) $(CSTARTOBJ) $(COBJS) $(CBIN)

realclean :
	rm -f $(OBJS) $(ORANGESBOOT) $(ORANGESKERNEL) $(SYMKERNEL) $(CSTARTOBJ) $(COBJS) $(CBIN)

disasm :
	$(DASM) $(DASMFLAGS) $(ORANGESKERNEL) > $(DASMOUTPUT)

buildimg :
	dd if=command/inst.tar of=$(HD) seek=$(shell echo "obase=10;ibase=16;(`egrep -e '^ROOT_BASE' boot/include/load.inc | sed -e 's/.*0x//g'`+`egrep -e '#define[[:space:]]*INSTALL_START_SECT' include/sys/config.h | sed -e 's/.*0x//g'`)*200" | bc) bs=1 count=$(shell ls -l command/inst.tar | awk -F " " '{print $$5}') conv=notrunc
	dd if=boot/boot.bin of=$(FD) bs=512 count=1 conv=notrunc
	dd if=boot/hdboot.bin of=$(HD) bs=1 count=446 conv=notrunc
	dd if=boot/hdboot.bin of=$(HD) seek=510 skip=510 bs=1 count=2 conv=notrunc
	sudo mount -o loop $(FD) /mnt/floppy/
	sudo cp -fv boot/loader.bin /mnt/floppy/
	sudo cp -fv kernel.bin /mnt/floppy
	sudo umount /mnt/floppy

ccommand :
	tar -cvf command/inst.tar -C command/ $(notdir $(CBIN))

boot/boot.bin: boot/boot.asm boot/include/load.inc boot/include/fat12hdr.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/hdboot.bin: boot/hdboot.asm boot/include/load.inc boot/include/fat12hdr.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin: boot/loader.asm boot/include/load.inc boot/include/fat12hdr.inc boot/include/pm.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/hdloader.bin: boot/hdloader.asm boot/include/load.inc boot/include/fat12hdr.inc boot/include/pm.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(ORANGESKERNEL): $(OBJS) $(LIB)
	$(LD) $(LDFLAGS) -o $(ORANGESKERNEL) $^

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(SRCASM:%.asm=%.o): %.o: %.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

$(SRCC:%.c=%.o): %.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

$(CSTARTOBJ): $(CSTART)
	$(ASM) $(CASMFLAGS) -o $@ $<

$(COBJS): %.o: %.c
	$(CC) $(CCFLAGS) -o $@ $<

$(CBIN): %: %.o $(CSTARTOBJ) $(LIB)
	$(LD) $(CLDFLAGS) -o $@ $< $(CSTARTOBJ) $(LIB)
