bits 16

mov ax, ds
mov [bx], cs
mov es, bx
mov ss, [1234h]
