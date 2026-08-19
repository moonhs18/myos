CC = gcc
LD = ld

CFLAGS = -m32 \
         -ffreestanding \
         -fno-pie \
         -fno-stack-protector \
         -nostdlib \
         -nostdinc \
         -Wall \
         -Wextra

all: myos.iso

boot.o:
	$(CC) -m32 -c boot/boot.s -o boot.o

kernel.o:
	$(CC) $(CFLAGS) -c kernel/kernel.c -o kernel.o

kernel.bin: boot.o kernel.o
	$(LD) -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o

myos.iso: kernel.bin
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/kernel.bin
	cp grub/grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso iso

run: myos.iso
	qemu-system-i386 -cdrom myos.iso

clean:
	rm -f boot.o kernel.o kernel.bin myos.iso
	rm -f iso/boot/kernel.bin
	rm -f iso/boot/grub/grub.cfg