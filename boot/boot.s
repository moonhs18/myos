.section .multiboot
.align 4

.long 0x1BADB002
.long 0
.long -(0X1BADB002)

.section .bss
.align 16

stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.type _start, @function

_start:
	mov $stack_top, %esp

	push %ebx
	push %eax

	call kernel_main

halt:
	cli
	hlt
	jmp halt

