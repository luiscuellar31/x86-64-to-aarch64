; Prueba valida: prologo, stack y epilogo canonico
inicio:
    push rbp
    mov rbp, rsp
    push rbx
    mov rbx, 10
    mov rax, rbx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret
