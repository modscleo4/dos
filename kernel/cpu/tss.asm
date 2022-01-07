global tss_flush

section .text
tss_flush:
   mov ax, (5 * 8) | 3
   ltr ax
   ret
