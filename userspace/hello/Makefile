CC     := gcc
AS     := nasm
CFLAGS := -m64 -ffreestanding -nostdlib -nostdinc \
          -O2 -I../libc/include \
          -mno-red-zone -mno-sse -mno-sse2 \
          -fno-stack-protector
LDFLAGS := -m elf_x86_64 -nostdlib -T linker.ld
LIBC    := ../libc/libc.a

all: hello.elf

hello.elf: hello.o $(LIBC)
	ld $(LDFLAGS) -o $@ $^

hello.o: hello.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f hello.o hello.elf
