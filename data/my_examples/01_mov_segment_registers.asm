bits 16

; Segegment register to register 16
mov ax, ds

; Segegment register to memory 16
mov [bx], cs
mov [si + 18], ds   ; 8bit displacement
mov [bp + 4460], ss ; 16bit displacement

; Register 16 to segment register
mov es, bx

; Memory to segment register
mov ss, [4460]
mov ss, [bx + 18]   ; 8bit displacement
mov ss, [bx + 4460] ; 16bit displacement
