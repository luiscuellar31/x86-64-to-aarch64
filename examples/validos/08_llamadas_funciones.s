; Prueba valida: llamada a subrutina sencilla con paso de argumentos y retorno
inicio:
    mov rdi, 10
    mov rsi, 20
    call sumar
    ret

sumar:
    add rdi, rsi
    mov rax, rdi
    ret
