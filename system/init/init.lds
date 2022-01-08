/* OUTPUT_FORMAT(elf32-i386); */
/* OUTPUT_ARCH(i386); */

ENTRY(init);

SECTIONS
{
    .text 0x101000 :
    {
        . = 0x0;
        *(.text.main)
        *(.text*)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }
}
