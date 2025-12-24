ENTRY(_start)

SECTIONS {
    . = 1M;
    _multiboot_start = .;

    .multiboot : {
        *(.multiboot)
    }

    .text : {
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    /DISCARD/ : {
        *(.comment)
        *(.note.gnu.property)
    }

    _bss_end   = .;
}
