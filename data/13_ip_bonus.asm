; ========================================================================
; LISTING 50
; ========================================================================

bits 16

mov ax, 10
mov bx, 10
mov cx, 10

label_0:
cmp bx, cx
je label_1

add ax, 1
jp label_2

label_1:
sub bx, 5
jb label_3

label_2:
sub cx, 2

label_3:
loopnz label_0
