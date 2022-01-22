OUTPUT(bootloader);

SECTIONS
{
    .text 0x0600:
    {
        /*. = 0x0;*/
        *(.text)
    }

    .data :
    {
        *(.data)
    }
}
