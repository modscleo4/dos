global load_idt

section .text
load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret
