; Bucle de cuatro iteraciones con comparacion explicita antes del salto.
mov rcx, 4
mov rax, rdi
bucle:
add rax, rsi
sub rcx, 1
cmp rcx, 0
jne bucle
ret
