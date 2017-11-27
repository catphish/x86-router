#!/bin/bash
set -e

rm -f *.o
nasm -f elf32 boot.asm
~/i686-elf-4.9.1-Linux-x86_64/bin/i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
~/i686-elf-4.9.1-Linux-x86_64/bin/i686-elf-gcc -c pci.c -o pci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
~/i686-elf-4.9.1-Linux-x86_64/bin/i686-elf-gcc -c debug.c -o debug.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
~/i686-elf-4.9.1-Linux-x86_64/bin/i686-elf-gcc -c nic.c -o nic.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
~/i686-elf-4.9.1-Linux-x86_64/bin/i686-elf-gcc -c malloc.c -o malloc.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
ld -m elf_i386 -n -o kernel.bin -T linker.ld *.o
scp kernel.bin 10.0.2.49:kernel.bin
ssh 10.0.2.49 sudo kvm -kernel kernel.bin -m 64M -chardev stdio,id=mydebug -device isa-debugcon,chardev=mydebug -display none -device pci-assign,host=08:00.0
