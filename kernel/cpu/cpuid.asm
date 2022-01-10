global cpuid_available

section .text
cpuid_available:
    pushfd                               ; Save EFLAGS
    pushfd                               ; Store EFLAGS
    xor dword [esp], 0x00200000          ; Invert the ID bit in stored EFLAGS
    popfd                                ; Load stored EFLAGS (with ID bit inverted)
    pushfd                               ; Store EFLAGS again (ID bit may or may not be inverted)
    pop eax                              ; eax = modified EFLAGS (ID bit may or may not be inverted)
    xor eax, [esp]                       ; eax = whichever bits were changed
    popfd                                ; Restore original EFLAGS
    and eax, 0x00200000                  ; eax = zero if ID bit can't be changed, else non-zero
    shr eax, 21                          ; get bit 21
    ret
