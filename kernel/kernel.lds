/* OUTPUT_FORMAT(elf32-i386); */
/* OUTPUT_ARCH(i386); */

ENTRY(swtich_to_pm);

SECTIONS
{
    .text 0x1000 :
    {
        . = 0x0;
        *(.text.swtich_to_pm)
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

    .bss :
    {
        *(.bss)
    }
}
