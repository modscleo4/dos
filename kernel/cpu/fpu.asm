global fpu_available

section .text
fpu_available:
    fninit
    fnstsw [.testword]
    cmp word [.testword], 0
    jne .nofpu
    fnstcw [.testword]
    mov ax, [.testword]
    and ax, 0x103F
    cmp ax, 0x003f
    jne .nofpu

    mov eax, 1
    ret

    .nofpu:
        mov eax, 0
        ret

.testword dw 0x55AA
