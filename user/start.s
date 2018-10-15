; start.asm

[BITS 32]
extern main
extern _exit

mov esp, 0x190000

   
   call main
      
  call _exit
    jmp  $
