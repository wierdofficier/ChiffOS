STARTUP(/home/kosmick/chiffos/ChiffOS/user/start.o)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
SECTIONS
{
    . = 0x1400000;
   .text : { *(.text) }
        .data : { *(.data .rodata) QUAD(ADDR(.bss)+SIZEOF(.bss)) }
   __bss_start = .;
        .bss : { *(.bss) }
        end = .; _end = .; __end = .;
}


