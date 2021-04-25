global tss_flush

section .text
tss_flush:
   mov ax, 0x2B
   ltr ax
   ret
