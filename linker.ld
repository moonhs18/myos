ENTRY(_start)

SECTIONS
{
    . = 0x40000000;


    .text.boot :
    {
        *(.text.boot)
    }
    .text : ALIGN(4K)
    {
        *(.text*)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata*)
    }

    .data : ALIGN(4K)
    {
        *(.data*)
    }

    .bss : ALIGN(16)
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(16);
        __bss_end = .;
    }

    .stack : ALIGN(16)
    {
        __stack_bottom = .;
        . = . + 0x10000;
        __stack_top = .;
    }

    __kernel_end = .;
}