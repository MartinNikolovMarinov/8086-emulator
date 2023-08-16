bits 16

add al, 0xff
add ax, 0xff00
mov byte [5], al
mov word [5], ax
sub al, 0xff
sub ax, 0xff00
mov al, byte [5]
mov ax, word [5]
add ax, 1 ; should flip everything to zero
mov word [5], ax
