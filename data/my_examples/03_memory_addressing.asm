bits 16

; Dirrect Addressing

mov word [1000], 0x0501
mov word [1003], 0x0602
mov word [1005], 0x0703
mov word [1006], 0x0004

mov byte [1001], 0x00
mov byte [1003], 0x00
mov byte [1004], 0x00
mov byte [1005], 0x00

mov byte [1002], 0x02
mov byte [1004], 0x03

; Memory should look like this ->
; [1000] == 0x0100
; [1002] == 0x0200
; [1004] == 0x0300
; [1008] == 0x0400

; Effective Address Calculation Addressing

mov si, 1000
mov word [si], 0x11
mov di, 1000
mov byte [di + 1], 0x22
mov bp, 1001
mov word [bp + 1], 0x3333
mov bx, 1006
mov word [bx - 2], 0x4444
mov bx, 1007
mov byte [bx - 1], 0x44
mov bx, 2007
mov word [bx - 1000], 0x4444 ; 16 bit negative displacement

; HARDCORE PARKOUR

mov bx, 1500
mov si, -500
mov byte [bx + si + 9], 0x55
mov bx, -500
mov di, 1509
mov byte [bx + di + 1], 0x66
mov bp, 1013
mov si, 0
mov word [bp + si - 2], 0x7777
mov bp, 1000
mov di, 13
mov byte [bp + di], 0x88

; Memory should look like this ->
; [1000] == 0x11
; [1001] == 0x22
; [1002] == 0x33
; [1003] == 0x33
; [1004] == 0x44
; [1005] == 0x44
; [1006] == 0x44
; [1007] == 0x44
; [1008] == 0x44
; [1009] == 0x55
; [1010] == 0x66
; [1011] == 0x77
; [1012] == 0x77
; [1013] == 0x88

; Reading from memory

mov word [1000], 0x1122
mov word [1002], 0x3344
mov word [1004], 0x5566
mov word [1006], 0x7788
mov word [1008], 0x99aa
mov word [1010], 0xbbcc
mov word [1012], 0xddee

; Memory should look like this ->
; [1000] == 0x22
; [1001] == 0x11
; [1002] == 0x44
; [1003] == 0x33
; [1004] == 0x66
; [1005] == 0x55
; [1006] == 0x88
; [1007] == 0x77
; [1008] == 0xaa
; [1009] == 0x99
; [1010] == 0xcc
; [1011] == 0xbb
; [1012] == 0xee
; [1013] == 0xdd

mov cx, word [1000] ; cx == 0x1122
mov dl, byte [1002] ; dx == 0x0044
mov dh, byte [1004] ; dx == 0x6644
mov bl, byte [1006] ; bx == 0x0088
mov bh, byte [1008] ; bx == 0xaa88

mov si, 1001
mov cx, [si] ; cx == 0x4411
mov bx, 0
mov di, 1003
mov bh, byte [di - 2] ; bx == 0x1100
mov bl, byte [di - 1] ; bx == 0x1144
mov cx, 0
mov bx, 1000
mov cx, word [bx + 8] ; cx == 0x99aa

; HARDCORE PARKOUR

mov dx, 0
mov bx, 1500
mov si, -500
mov dx, [bx + si + 10] ; dx == 0xbbcc
mov bp, 1013
mov si, 0
mov dx, [bp + si - 2] ; dx == 0xeebb
mov dx, 0
mov bp, 1000
mov di, 12
mov dh, byte [bp + di] ; dx == 0xee00
mov dl, byte [bp + di + 1] ; dx == 0xeedd

