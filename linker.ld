ENTRY(_start)

SECTIONS
{
    . = 0x40000000;


    .text.boot : ALIGN(4)
    {
        *(.text.boot)
    }
    . = ALIGN(2048);

    .text.vector :
    {
        *(.text.vector)
    }

    . = ALIGN(4096);
    
    .text :
    {
        __kernel_text_start = .;
        *(.text*)
        __kernel_text_end = .;
    }

    . = ALIGN(4096);


    .rodata :
    {
        __kernel_rodata_start = .;
        *(.rodata*)
        __kernel_rodata_end = .;
    }

    . = ALIGN(4096);

    .data :
    {
        __kernel_data_start = .;
        *(.data*)
        __kernel_data_end = .;
    }

    . = ALIGN(16);

    .bss :
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(16);
        __bss_end = .;
    }
    
    . = ALIGN(16);


    .stack :
    {
        __stack_bottom = .;
        . = . + 0x10000;
        __stack_top = .;
    }
    
    . = ALIGN(4096);
    __kernel_end = .;

    . = 0x47000000;
    
    .user_text :
    {
        
        . = ALIGN(4096);
        __user_text_start = .;
        KEEP(*(.user_text*))
        . = ALIGN(4096);
        __user_text_end = .;
    }

    .user_rodata :
    {
        . = ALIGN(4096);
        __user_rodata_start = .;
        KEEP(*(.user_rodata*))
        . = ALIGN(4096);
        __user_rodata_end = .;
    }

    .user_data :
    {
        . = ALIGN(4096);
        __user_data_start = .;
        KEEP(*(.user_data*))
        . = ALIGN(4096);
        __user_data_end = .;
    }

    __user_image_start = __user_text_start;
    __user_image_end = .;
}