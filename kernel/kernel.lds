/* OUTPUT_FORMAT(elf32-i386); */
/* OUTPUT_ARCH(i386); */

ENTRY(_start);

SECTIONS
{
    . = 0x100000;

    PROVIDE(kernel_start_real_addr = .);

    .boot ALIGN(8) :
    {
        *(.multiboot2_header)
    }

    .text.setup ALIGN(0x1000) :
    {
        *(.text.setup)
    }

    .bss.setup ALIGN(0x1000) :
    {
        *(.bss.setup)
    }

    . += 0xC0000000;

    PROVIDE(kernel_start_addr = .);

    .text ALIGN(0x1000) : AT(ADDR(.text) - 0xC0000000)
    {
        *(.text)
    }

    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata)
    }

    .data ALIGN(0x1000) : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
    }

    .debug_line ALIGN(0x1000) : AT(ADDR(.debug_line) - 0xC0000000)
    {
        *(.debug_line)
    }

    .bss ALIGN(0x1000) : AT(ADDR(.bss) - 0xC0000000)
    {
        *(.bss)
    }

    PROVIDE(kernel_end_addr = .);

    . -= 0xC0000000;

    PROVIDE(kernel_end_real_addr = .);
}
