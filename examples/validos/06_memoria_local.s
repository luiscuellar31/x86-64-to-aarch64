; Prueba valida: memoria local controlada en stack (rbp)
inicio:
    mov [rbp - 8], rax
    mov [rbp - 16], rbx
    mov rcx, [rbp - 8]
    mov rdx, [rbp - 16]
    add rcx, rdx
    mov [rbp - 24], rcx
    ret
