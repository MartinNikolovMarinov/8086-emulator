; ========================================================================
; LISTING 44
; ========================================================================

bits 16

mov ax, 1
mov bx, 2
mov cx, 3
mov dx, 4

mov sp, ax
mov bp, bx
mov si, cx
mov di, dx

mov dx, sp
mov cx, bp
mov bx, si
mov ax, di
