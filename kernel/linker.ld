ENTRY(_start)

SECTIONS
{
    /* Starts at LOADER_ADDR. */
    . = 0x80000;
    __start = .;
    __text_start = .;

    .text ALIGN(4K) : AT(ADDR(.text))
    {
        KEEP(*(.text.boot))
        *(.text)
    }
    __text_end = .;

    __rodata_start = .;
    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }
    __rodata_end = .;

    __data_start = .;
    .data ALIGN(4K) :
    {
        *(.data)
    }
    __data_end = .;

    __bss_start = .;
    .bss ALIGN(4K) :
    {
        bss = .;
        *(.bss)
    }

    . = ALIGN(4096);
    __bss_end = .;
    __bss_size = (__bss_end - __bss_start) >> 3;
    __end = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}