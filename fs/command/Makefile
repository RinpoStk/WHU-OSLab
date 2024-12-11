# commands/Makefile

#ENTRYPOINT	= 0x1000
HD		= ../80m.img

ASM		= nasm
DASM		= objdump
CC		= gcc
LD		= ld
ASMFLAGS	= -I ../include/ -f elf
CFLAGS		= -m32 -I ../include/ -c -fno-builtin -fno-stack-protector -Wall
LDFLAGS		= -m elf_i386 -Ttext 0x1000
DASMFLAGS	= -D
LIB		= ../lib/orangescrt.a
BIN		= echo pwd rm touch ps ls cat run test kill wait edit

# All Phony Targets
.PHONY : everything final clean realclean disasm all install

# Default starting position
everything : $(BIN)

install : all clean
	cp ../boot/hd*.bin ./ -v
	cp ../kernel.bin ./ -v
	tar vcf inst.tar kernel.bin $(BIN) hd*.bin
	dd if=inst.tar of=$(HD) seek=`echo "obase=10;ibase=16;(\`egrep -e '^ROOT_BASE' ../boot/include/load.inc | sed -e 's/.*0x//g'\`+\`egrep -e '#define[[:space:]]*INSTALL_START_SECT' ../include/sys/config.h | sed -e 's/.*0x//g'\`)*200" | bc` bs=1 count=`ls -l inst.tar | awk -F " " '{print $$5}'` conv=notrunc

all : realclean everything

final : all clean

clean :
	rm -f *.o

realclean :
	rm -f $(BIN) *.o

kernel.bin :
	cp ../kernel.bin ./

start.o : start.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

echo.o: echo.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

echo : echo.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

pwd.o: pwd.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

pwd : pwd.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

rm.o: rm.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

rm : rm.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

touch.o: touch.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

touch : touch.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

ps.o: ps.c ../include/type.h ../include/stdio.h 
	$(CC) $(CFLAGS) -o $@ $<

ps : ps.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

ls.o: ls.c ../include/type.h ../include/stdio.h 
	$(CC) $(CFLAGS) -o $@ $<

ls : ls.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

cat.o: cat.c ../include/type.h ../include/stdio.h 
	$(CC) $(CFLAGS) -o $@ $<

cat : cat.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

run.o: run.c ../include/type.h ../include/stdio.h 
	$(CC) $(CFLAGS) -o $@ $<

run : run.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

test.o: test.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

test : test.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

kill.o: kill.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

kill : kill.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

wait.o: wait.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

wait : wait.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

edit.o: edit.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

edit : edit.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?