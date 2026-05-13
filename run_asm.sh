nasm -f elf64 OUTPUT.asm -o output.o
ld output.o -o output
./output
