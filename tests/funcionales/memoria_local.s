; Variables locales dentro de un marco de pila reservado y restablecido.
push rbp
mov rbp, rsp
sub rsp, 16
mov [rbp - 8], rdi
mov [rbp - 16], rsi
mov rax, [rbp - 8]
mov rdx, [rbp - 16]
add rax, rdx
mov rsp, rbp
pop rbp
ret
