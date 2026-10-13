; Llamadas internas sucesivas y anidadas con stack balanceado.
; Resultado esperado en rax: (7 * 2) + 1 = 15.
inicio:
    mov rax, 7
    call duplicar
    call incrementar
    ret

duplicar:
    push rbp
    mov rbp, rsp
    call doblar
    mov rsp, rbp
    pop rbp
    ret

doblar: add rax, rax
    ret

incrementar: add rax, 1
    ret
