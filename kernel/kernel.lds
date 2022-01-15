/* OUTPUT_FORMAT(elf32-i386); */
/* OUTPUT_ARCH(i386); */

ENTRY(kernel_start);

SECTIONS
{
    .text 0x11000 :
    {
        . = 0x0;
        *(.text.kernel_start)
        *(.text.switch_ring3)
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
