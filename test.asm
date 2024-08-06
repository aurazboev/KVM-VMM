; test.asm
BITS 32

    mov dx,0x3f8          ; COM1 serial port
    add al, bl
    mov al, 2
    add al, 2              ; Add 2 to AL
    add al, '0'            ; Convert the number to its ASCII representation
    out dx, al             ; Output AL to the serial port (COM1)
    mov al, 0x0a           ; Load the newline character into AL
    out dx, al             ; Output AL to the serial port (COM1)
    hlt                    ; Halt the CPU

