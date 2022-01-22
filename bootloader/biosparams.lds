OUTPUT(biosparams);

SECTIONS
{
    .text 0x0:
    {
        /*. = 0x0;*/
        *(.text)
    }
}
