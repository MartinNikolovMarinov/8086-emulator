; ========================================================================
;
; (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
;
; Please see https://computerenhance.com for further information
;
; ========================================================================

; ========================================================================
; LISTING 41
; ========================================================================

;
; NOTE(casey): This is not meant to be a real compliance test for 8086
; disassemblers. It's just a reasonable selection of opcodes and patterns
; to use as a first pass in making sure a disassembler handles a large
; cross-section of the encoding. To be absolutely certain you haven't
; missed something, you would need a more exhaustive listing!
;

bits 16

add bx, [bx + si]
add bx, [bp]
add si, 2
add bp, 2
add cx, 8
add bx, [bp]
add cx, [bx + 2]
add bh, [bp + si + 4]
add di, [bp + di + 6]
add [bx + si], bx
add [bp], bx
add [bp], bx
add [bx + 2], cx
add [bp + si + 4], bh
add [bp + di + 6], di
add byte [bx], 34
add word [bp + si + 1000], 29
add ax, [bp]
add al, [bx + si]
add ax, bx
add al, ah
add ax, 1000
add al, -30
add al, 9

sub bx, [bx + si]
sub bx, [bp]
sub si, 2
sub bp, 2
sub cx, 8
sub bx, [bp]
sub cx, [bx + 2]
sub bh, [bp + si + 4]
sub di, [bp + di + 6]
sub [bx + si], bx
sub [bp], bx
sub [bp], bx
sub [bx + 2], cx
sub [bp + si + 4], bh
sub [bp + di + 6], di
sub byte [bx], 34
sub word [bx + di], 29
sub ax, [bp]
sub al, [bx + si]
sub ax, bx
sub al, ah
sub ax, 1000
sub al, -30
sub al, 9

cmp bx, [bx + si]
cmp bx, [bp]
cmp si, 2
cmp bp, 2
cmp cx, 8
cmp bx, [bp]
cmp cx, [bx + 2]
cmp bh, [bp + si + 4]
cmp di, [bp + di + 6]
cmp [bx + si], bx
cmp [bp], bx
cmp [bp], bx
cmp [bx + 2], cx
cmp [bp + si + 4], bh
cmp [bp + di + 6], di
cmp byte [bx], 34
cmp word [4834], 29
cmp ax, [bp]
cmp al, [bx + si]
cmp ax, bx
cmp al, ah
cmp ax, 1000
cmp al, -30
cmp al, 9

jne label_0
jne label_1
jne label_2
jne label_3
label_0:
cmp word [4834], 29
jne label_0
label_1:
cmp [bp + si + 4], bh
jne label_0
cmp [bp + si + 4], bh
cmp ax, bx
cmp al, ah
jne label_1
cmp word [4834], 29
jne label_2
cmp [bp + si + 4], bh
jne label_1
cmp [bp + si + 4], bh
label_2:
jne label_2
cmp word [4834], 29
jne label_1
jne label_0
label_3:
jne label_4
label_4:

je label_5
label_5:
jl label_5
jle label_5
jb label_5
jbe label_5
jp label_5
jo label_5
js label_5
jne label_5
jnl label_5
jnle label_5
jnb label_5
jnbe label_5
jnp label_5
jno label_5
jns label_5
loop label_5
loope label_5
loopne label_5
jcxz label_5
