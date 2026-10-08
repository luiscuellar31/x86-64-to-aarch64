; Prueba valida: operaciones logicas a nivel de bits (and, or, xor)
inicio:
    mov rax, 255
    and rax, 15
    mov rbx, 16
    or rax, rbx
    xor rax, rax
    ret
