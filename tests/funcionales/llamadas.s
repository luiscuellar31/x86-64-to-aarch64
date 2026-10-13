; Dos niveles de llamadas internas antes de sumar el segundo argumento.
mov rax, rdi
call incrementar_y_duplicar
add rax, rsi
ret
incrementar_y_duplicar:
call sumar_uno
add rax, rax
ret
sumar_uno:
add rax, 1
ret
