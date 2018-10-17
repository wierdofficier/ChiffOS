; start.asm

[BITS 32]
extern main
extern _exit
extern environ
mov esp, 0x190000
mov eax, DWORD  [esp+4]
mov ebx, DWORD  [esp+8]
 
push ecx
push edx
 
;mov environ, ecx  ;nån slags extern lär ju krävas också, vet inte syntax för den assemblern
call main
add esp, 8
push eax
call _exit
   	jmp  $
