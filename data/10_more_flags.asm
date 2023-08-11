; ========================================================================
; LISTING 47
; ========================================================================

bits 16

add bx, 30000
add bx, 10000
sub bx, 5000
sub bx, 5000

mov bx, 1
mov cx, 100
add bx, cx

mov dx, 10
sub cx, dx

add bx, 40000
add cx, -90

mov sp, 99
mov bp, 98
cmp bp, sp
