/* OUTPUT_FORMAT(elf32-i386); */
/* OUTPUT_ARCH(i386); */

ENTRY(kernel_start);

SECTIONS
{
    . = 0x100000;

    .boot :
    {
        *(.multiboot2_header)
    }

    .text :
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }
}
