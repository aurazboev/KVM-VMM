BITS 32

section .text
global _start

_start:
loop_start:
    mov dx, 0x45        ; Set DX to 0x45
    add al, bl          ; Add BL to AL
    in al, dx           ; Read from port DX into AL

    sub dx, 3           ; Subtract 3 from DX
    out dx, al          ; Write AL to port DX

    ; Replace 'hlt' with a jump to 'loop_start' to continuously loop
    jmp loop_start     ; Jump back to the beginning of the loop

