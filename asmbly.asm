section .text
global _start

_start:
    add al, bl
    add bl, al
    add cl, al
    mov dx, 0x42

.loop:
    cmp bl, 1
    jne .read
    out dx, al
    dec bl

.read:
    add dx, 2
    in al, dx
    cmp al, 0x00
    je .equal
    add dx, 1
    mov cl, al
    in al, dx
    mov bl, al
    mov al, cl
    sub dx, 3
    jmp .continue

.equal:
    sub dx, 2

.continue:
    jmp .loop
