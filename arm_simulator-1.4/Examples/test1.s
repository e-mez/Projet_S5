.global main
.text
main:
    mov r5, #0
    mov r6, #1
    add r7, r5, r6
    orr r8, r5, r6
    eor r9, r5, r6
    and r10, r5, r6
    mov r0, #0x12
    mov r1, #0x34
    add r0, r1, r0, lsl #8
    mov r1, #0x56
    add r0, r1, r0, lsl #8
    mov r1, #0x78
    add r0, r1, r0, lsl #8
    mov r1, #0x2000
    strb r0, [r1]
    ldrb r2, [r1]
    add r1, r1, #3
    ldrb r3, [r1]
    swi 0x123456
