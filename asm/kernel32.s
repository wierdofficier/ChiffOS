 global loader:function  ; making entry point visible to linker
extern kmain            ; kmain is defined elsewhere

; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0                   ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                   ; provide memory map
FLAGS       equ  MODULEALIGN | MEMINFO  ; this is the Multiboot 'flag' field
MAGIC       equ    0x1BADB002           ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)        ; checksum required

section .__mbHeader ; to keep GRUB happy, this needs to be first; see linker.ld
align 4

MultiBootHeader:
   dd MAGIC
   dd FLAGS
   dd CHECKSUM

section .text
align 4

; reserve initial kernel stack space
STACKSIZE equ 0x4000

loader:
   mov esp, stack+STACKSIZE           ; set up the stack
   push stack                         ; pass the initial stack ESP0
   push eax                           ; pass Multiboot magic number
   push ebx                           ; pass Multiboot info structure

   call  kmain                       ; call the kernel

   cli
	.hang:
	   hlt                                ; halt machine should kernel return
	   jmp   .hang


;imports
 
extern gdtpointer
extern idtpointer

extern irq_handler
extern kputs

;exports
global idt_flush
global gdt_flush
global tss_flush
global kputhex
global kputint

	
gdt_flush:
	lgdt [gdtpointer] 
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	jmp 0x08:flush
flush:
	ret ; returns back to C-code

idt_flush:
	lidt [idtpointer]
	ret
	
tss_flush:
	mov ax, 0x2b
	ltr ax
	ret

; IRQ handling
text:

%macro IRQ 2 
[GLOBAL irq%1]
irq%1:
cli
push byte 0
push byte %2
jmp irq_common_stub
%endmacro



IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47
IRQ 126, 126 ; the task switch vector
IRQ 127, 127 ; the task switch vector 
irq_common_stub:
	push eax
    push ecx
    push edx
    push ebx
    
    push ebp
    push esi
    push edi

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

	push esp
	call irq_handler
	mov esp, eax

	pop gs
    pop fs
    pop es
    pop ds

    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax

    add esp, 8
	sti
    iret

.hang:
	jmp .hang


;***************************************
;--puthex-- 
; In: eax
; Return: none
;***************************************
section .bss

hexkod: resb 16
section .text

kputhex:
pusha
	lea esi, [hexkod + 14] ;lagra arrayens adress i esi, vi räknar sedan från höger till vänster
.loop:
	xor edx, edx
	mov ebx, 16
	div ebx		;tecknet finns i edx, är det över eller under 10?
	mov byte [esi], dl	;lagra dl i arrayen, dvs esi pekar på arrayen
	cmp dl, 0xa
	jge .bokstaver
	
	add byte [esi], 0x30	;börja från offset 0x30
	jmp .end		;det är ett tecken i taget
.bokstaver:
	mov byte [esi], dl
	add byte [esi], 'A' - 10
.end:
	dec esi			;vi arbetar till vänster
	cmp eax, 0		;är numret 0, isf är dl ochså 0
	jne .loop

		
    dec esi			
	mov byte [esi], '0'
	mov byte [esi +1], 'x'
	push esi
	;call kputs ;läser in adressen esi och printar ut dess innehåll
	pop esi
	popa 
	ret
;***************************************
;--putint-- 
; In: eax
; Return: none
;***************************************
section .bss
numstr: resb 16
section .text
kputint:


lea esi, [numstr + 14] ; ladda in adressen till slutet av numstr, lämna 1 byte för NULL
.loop:
	xor edx, edx ; töm EDX, används av div annars
	mov ebx, 0xa ; div kan inte ta numret direkt
	div ebx
	; nu är eax = talet / 10, medans edx är sista siffran som var kvar, dvs den som vi vill ha
	
	mov byte [esi], dl ; skriv siffran (lagrad i dl, eftersom resten av edx är nollor) till arrayen i RAM
	add byte [esi], 0x30 ;Börja från offset 0x30, konvertera från t ex numret 5 till siffran '5' i ASCII, 0x30 = '0', 0x31 = '1' osv

	dec esi ; flytta pekaren ett steg närmare början, dvs jobba till vänster eftersom vi börjar på adressen längst till höger i arrayen
	cmp eax, 0 ; är numret 0 efter divisionen? dvs om numret är 0 så finns inga remainders.
	jne .loop ; om inte, loopa

	;.end:
	; numret är 0 nu, så skriv ut siffrorna som blev

	; skriv null till slutet av strängen
	;lea ebx, [numstr + 15]
	;mov byte [ebx], 0

	inc esi ;esi innehåller strängen men [] behövs itne då den användes med lea, vi gick bak 1 byte för långt förut, och pekar FÖRE strängen!
	push esi
	;call kputs
	pop esi
	ret

%macro ISR_NOERRCODE 1
[GLOBAL isr%1]
isr%1:
cli
push 0
push %1
jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
[GLOBAL isr%1]
isr%1:
cli
push %1
jmp isr_common_stub
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE 17 
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31
ISR_NOERRCODE 32
; the syscall handler
ISR_NOERRCODE 127



extern isr_handler

isr_common_stub:
push eax
    push ecx
    push edx
    push ebx
    
    push ebp
    push esi
    push edi

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

	push esp
	call isr_handler
	mov esp, eax

	pop gs
    pop fs
    pop es
    pop ds

    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax

    add esp, 8
sti
    iret

section .bss
align 4
stack:
resb STACKSIZE ; reserve 16k stack on a doubleword boundary
