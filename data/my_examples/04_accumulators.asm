bits 16

add al, 5
add ah, 6
sub al, 5
sub ah, 6

mov word [1000], 0xffff
mov al, [1000]
mov ah, [1000+1]
mov ax, [0] ; zero out ax

mov byte [1000], al ; should clear from 1000 to 1001
mov word [1000], ax ; should clear from 1000 to 1002

mov word [1000], 0x1122
mov si, 1000
mov ah, [si]
mov al, [si+1]

mov ah, 0
mov si, 1000
mov word [si], ax


