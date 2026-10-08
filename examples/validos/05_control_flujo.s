; Prueba valida: comparacion y control de flujo condicional/incondicional
inicio:
    mov rax, 10
    mov rbx, 20
    cmp rax, rbx
    jl es_menor
    jmp fin

es_menor:
    mov rax, 1

fin:
    ret
