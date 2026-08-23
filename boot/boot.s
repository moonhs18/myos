.section ".text.boot"

.global _start

_start:
	mrs x0, mpidr_el1
	and x0, x0, #0xFF
	cbz x0, master_core

hang:
	wfe
	b hang

master_core:
	ldr x1, =_bss_start
	ldr x2, =_bss_end

bss_clear_loop:
	cmp x1,x2
	b.ge init_stack
	str xzr, [x1], #8
	b bss_clear_loop

init_stack:
	ldr x0, =_start
	mov sp, x0

	bl kernel_main

halt:
	wfi
	b halt
