all: gemOS.kernel
SRCS = entry.c mmap.c 
OBJS = entry.o mmap.o
OBJSALL = boot.o main.o lib.o idt.o kbd.o shell.o serial.o memory.o context.o entry.o apic.o schedule.o mmap.o page.o file.o entry_helpers.o hugepage.o 
CFLAGS  = -g -nostdlib -nostdinc -fno-builtin -fno-stack-protector -fpic -m64 -I./include -I../include 
LDFLAGS = -nostdlib -nodefaultlibs  -q -melf_x86_64 -Tlink64.ld
ASFLAGS = --64  
AS = as 
USER_CFLAGS  = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -fpic -m64 -I./user/

%o: %c
	gcc -c $(CFLAGS) $< -o $@ 

boot.o : boot_64.S
	$(AS) $(ASFLAGS) $^ -o $@


user/lib.o:user/lib.c user/ulib.h
	gcc -c $(USER_CFLAGS) user/lib.c -o user/lib.o
user/init.o:user/init.c user/ulib.h
	gcc -c $(USER_CFLAGS) user/init.c -o user/init.o

gemOS.kernel: $(OBJS) user/init.o user/lib.o
	ld $(LDFLAGS) -o $@ $(OBJSALL) user/init.o user/lib.o

.Phony: clean
clean:
	rm -f $(OBJS); rm -f gemOS.kernel; rm -f user/*.o;
