typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

#define VGA_MEMORY ((uint16_t*)0xB8000)

void kernel_main(uint32_t magic, uint32_t multiboot_info)
{
    (void)magic;
    (void)multiboot_info;

    const char *message = "Hello from MyOS!";

    uint8_t color = 0x0F;

    for (int i = 0; message[i] != '\0'; i++)
    {
        VGA_MEMORY[i] =
            (uint16_t)message[i] |
            ((uint16_t)color << 8);
    }

    while (1)
    {
        __asm__ volatile ("hlt");
    }
}