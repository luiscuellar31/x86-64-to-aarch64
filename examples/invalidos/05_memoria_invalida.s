; Prueba invalida: transferencia memoria a memoria no soportada
inicio:
    mov [rbp - 8], [rbp - 16]
    ret
