; Resultado -1, 0 o 1 segun el orden con signo de los argumentos.
cmp rdi, rsi
je iguales
jl menor
mov rax, 1
ret
iguales:
mov rax, 0
ret
menor:
mov rax, -1
ret
