#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ELF_MAGIC   0x464C457F
#define ET_EXEC     2
#define EM_X86_64   62
#define PT_LOAD     1
#define PF_X        0x1
#define PF_W        0x2
#define PF_R        0x4

typedef struct __attribute__((packed)) {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} elf64_ehdr_t;

typedef struct __attribute__((packed)) {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} elf64_phdr_t;

typedef struct {
    uint64_t  entry;
    uint64_t  load_end;
    bool      valid;
} elf_info_t;

bool       elf_validate(const void *data, size_t size);
elf_info_t elf_load(uint64_t *pml4, const void *data, size_t size);
