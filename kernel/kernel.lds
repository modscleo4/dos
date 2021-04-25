/* OUTPUT_FORMAT(elf32-i386); */
/* OUTPUT_ARCH(i386); */

ENTRY(kernel_main);

SECTIONS
{
    .text 0x1000 :
    {
        . = 0x0;
        *(.text.kernel_start)
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
