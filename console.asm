BITS 32

section .text
global _start

_start:
    mov dx, 0x42
    add al, bl
    mov al, 'H'
    out dx, al
    mov al, 'e'
    out dx, al
    mov al, 'l'
    out dx, al
    mov al, 'l'
    out dx, al
    mov al, 'o'
    out dx, al
    mov al, ','
    out dx, al
    mov al, ' '
    out dx, al
    mov al, 'W'
    out dx, al
    mov al, 'o'
    out dx, al
    mov al, 'r'
    out dx, al
    mov al, 'l'
    out dx, al
    mov al, 'd'
    out dx, al
    mov al, '!'
    out dx, al
    mov al, 0x0a
    out dx, al

    hlt

