OUTPUT(bootloader);

SECTIONS
{
    .text 0x7C00:
    {
        /*. = 0x0;*/
        *(.text)
    }

    .data :
    {
        *(.data)
    }
}
