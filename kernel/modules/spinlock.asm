global spinlock_test_and_set
global spinlock_release

section .text
spinlock_test_and_set:
    mov ecx, [esp+8]
    mov edx, [esp+4]
retry:
    xor eax, eax
    xacquire lock cmpxchg [edx], ecx
    je out
pause:
    mov eax, [edx]
    test eax, eax
    jz retry
    rep nop
    jmp pause
out:
    ret

    ;mov ecx, [esp+8]
    ;mov edx, [esp+4]
    ;mov eax, ecx
    ;xchg eax, [ecx]
    ;test eax, eax
    ;jnz spinlock_test_and_set
    ;ret

spinlock_release:
    mov edx, [esp+4]
    xrelease mov word [edx], 0
    ret

    ;mov ecx, [esp+4]
    ;xor eax, eax
    ;xchg eax, [ecx]
    ;ret
