WARNINGS := -Wall -ansi \
				 -Wextra -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes \
				 -Wmissing-declarations -Wnested-externs -Winline -Wno-long-long \
				 -Wstrict-prototypes -fno-builtin
				 
LDFLAGS =  -Tkernlink.ld
STARTDIR = /home/sim/githubCHIFFO/ChiffOS-master
NEWLIB = /home/sim/Hämtningar/toaruos-master
CFLAGS =     -O0 -nostdlib -nostdinc -finline-functions -ffreestanding -ffreestanding  -fno-strict-aliasing -fno-common -I$(STARTDIR)/kernel/h/  -I$(STARTDIR)/kernel/src/lwip/src/include/ -I$(STARTDIR)/kernel/src/lwip/src/include/ipv6/ -I$(STARTDIR)/kernel/src/lwip/src/include/ipv4/ -I$(STARTDIR)/kernel/src/lwip/lwip/src/include/     -I$(STARTDIR)/kernel/src/lwip/src/include -I$(STARTDIR)/kernel/src/lwip/src/include/ipv4 -I$(STARTDIR)/kernel/src/lwip/src/include/ipv6 -I$(NEWLIB)/toolchain/tarballs/newlib-1.19.0/newlib/libc/include
CC = i686-pc-epzordiun-gcc
LD = i686-pc-epzordiun-ld
NASMFLAGS = -Ox -f elf
RM = rm

PROJDIRS := kernel/src kernel/h asm/
SRCFILES := $(shell find $(PROJDIRS) -type f -name '*.c')
HDRFILES := $(shell find $(PROJDIRS) -type f -name '*.h')
ASMFILES := $(shell find $(PROJDIRS) -type f -name '*.s')
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
OBJFILES += $(patsubst %.s,%.o,$(ASMFILES))

USERSPACEPROG := $(shell find user/ -maxdepth 2 -name 'Makefile' -exec dirname {} \;)

.s.o:
	nasm $(NASMFLAGS) $<
all: $(OBJFILES) link
					
link:
	$(LD) $(LDFLAGS) -o KERNEL $(OBJFILES)  asm/task.o asm/user.o   
	qemu-system-i386 -sdl -no-frame -k en-us -m 2024 -vga std -rtc base=localtime -net nic,model=rtl8139 -net user -net dump -kernel KERNEL    -serial   		stdio -append "vid=qemu,,1280,,720 logtoserial=1 root=/dev/hda" -enable-kvm
 
clean:
	-$(RM) $(wildcard $(OBJFILES))
	-$(RM) KERNEL
	@for prog in $(USERSPACEPROG); do \
		make -C $$prog clean; \
		rm -f misc/`basename $$prog` ; \
	done


