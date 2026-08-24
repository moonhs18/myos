.section ".text.boot"
.global _start

_start:
	mrs x0, mpidr_el1
	and x0, x0, #0xFF
	cbz x0, master_core

hang:
	wfe
	b hang

el_setup:
	mrs x0, CurrentEL
	lsr x0, x0, #2

	cmp x0, #3
	b.eq level_el3
	cmp x0, #2
	b.eq level_el2
	cmp x0, #1
	b init_kernel_sp
	
level_el3:
	mov x1, #(1<<1024)
	msr scr_el3, x1

	mov x1, #0b00101
	msr spsr_el3, x1
	
	adr x1, init_kernel_sp
	msr elr_el3, x1
	
	eret

level_el2:
	mov x1, #(1<<31)
	msr hcr_el2, x1

	mov x1, #0b00101
	msr spsr_el2, x1
	
	adr x1, init_kernel_sp
	msr elr_el2, x1
	
	eret

init_kernel_sp:
	ldr x0, =_stack_top
	mov sp, x0

	ldr x1, = _bss_start
	ldr x2, = _bss_end

clear_bss:
	cmp x1, x2
	b.ge enter_kernel
	str xzr, [x1], #8
	b clear_bss

enter_kernel:
	bl kernel_main
system_halt:
	wfi
	b system_halt