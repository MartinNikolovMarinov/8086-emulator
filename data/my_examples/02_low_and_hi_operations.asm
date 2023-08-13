bits 16

mov ax, 0xeeff ; ax: 0xeeff
mov bx, ax     ; bx: 0xeeff
mov bx, 0      ; bx: 0x0000
mov bl, al     ; bx: 0x00ff
mov bh, ah     ; bx: 0xeeff
mov bx, 0      ; bx: 0x0000
mov bl, ah     ; bx: 0x00ee
mov bh, al     ; bx: 0xffee
mov bx, 0      ; bx: 0x0000
mov al, 0      ; ax: 0xee00
mov ax, 0xeeff ; ax: 0xeeff
mov ah, 0      ; ax: 0x00ff
mov ax, 0      ; ax: 0x0000

; All registers should be 0x0000

add bx, 0xeeff ; bx: 0xeeff , flags: PS
mov bx, 0      ; bx: 0x0000 , flags: PS
add bl, 0xff   ; bx: 0x00ff , flags: PS
add bh, 0xee   ; bx: 0xeeff, flags: PS
add cl, 1      ; cx: 0x0001 , flags: -
add cl, bl     ; cx: 0x0000 , flags: CPAZ
add ch, 0x12   ; cx: 0x1200 , flags: P
add ch, bh     ; cx: 0x0000 , flags: CPAZ
add cx, 0x1201 ; cx: 0x1201 , flags: -
add cx, bx     ; cx: 0x0100 , flags: CPA
add ch, bl     ; cx: 0x0000 , flags: CPAZ
add cl, 0x12   ; cx: 0x0012 , flags: P
add cl, bh     ; cx: 0x0000 , flags: CPAZ
add bh, 0x12   ; bx: 0x00ff , flags: CPAZ
add bl, 1      ; bx: 0x0000 , flags: CPAZ
add bl, 127    ; bx: 0x007f , flags: -
add bl, 1      ; bx: 0x0080 , flags: ASO
add bl, -128   ; bx: 0x0000 , flags: CPZO
add bh, 127    ; bx: 0x7f00 , flags: -
add bh, 1      ; bx: 0x8000 , flags: ASO
add bh, -128   ; bx: 0x0000 , flags: CPZO
add bx, 0x7FFF ; bx: 0x7fff , flags: P
add bx, 1      ; bx: 0x8000 , flags: PASO

; All registers should be 0x0000

mov bx, 0xeeff ; bx: 0xeeff, flags: -
sub bx, 0xeeff ; bx: 0x0000, flags: PZ
mov bx, 0xeeff ; bx: 0xeeff, flags: PZ
sub bl, 0xff   ; bx: 0xee00, flags: PZ
sub bh, 0xee   ; bx: 0x0000, flags: PZ
mov bx, 0x0102 ; bx: 0x0102, flags: PZ
mov cx, 0x0102 ; cx: 0x0102, flags: PZ
sub cx, bx     ; cx: 0x0000, flags: PZ
sub cl, 0x01   ; cx: 0x00ff, flags: CPAS
sub ch, 0x01   ; cx: 0xffff, flags: CPAS
sub cx, 1      ; cx: 0xfffe, flags: S
mov cx, 0      ; cx: 0x0000, flags: S
sub bl, -126   ; bx: 0x0180, flags: CSO
sub bh, -127   ; bx: 0x8080, flags: CSO
sub cl, bh     ; cx: 0x0080, flags: CSO
sub ch, bl     ; cx: 0x8080, flags: CSO
sub bh, ch     ; bx: 0x0080, flags: PZ
sub bl, cl     ; bx: 0x0000, flags: PZ
mov cx, 0      ; cx: 0x0000, flags: PZ

; All registers should be 0x0000
