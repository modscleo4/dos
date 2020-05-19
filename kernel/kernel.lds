/*   OUTPUT_FORMAT(elf32-i386); */
/* OUTPUT_ARCH(i386); */

ENTRY(main);

SECTIONS
{
    .text 0x1000:
    {
        . = 0x0;
        *(.text.main)
        *(.text*)
    }
    .data :
    {
        *(.data)
    }
    .bss 0x20000:
    {
        *(.bss)
    }
}
