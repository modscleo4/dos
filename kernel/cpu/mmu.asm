global mmu_enable_paging

section .text
mmu_enable_paging:
    mov eax, [esp + 4]
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax
    ret
