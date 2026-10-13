; Mascara del primer argumento, OR con el segundo y cambio de los bits bajos.
mov rax, rdi
and rax, 255
or rax, rsi
xor rax, 15
ret
