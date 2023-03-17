        .cpu cortex-m0
        .code   16
        .text
        .global HardFault_Handler
.align  2
.thumb_func
HardFault_Handler:
    ldr r0, .L1.landing_place
    ldr r0, [r0]
.L2:
    cmp r0, #0
    beq .L2
    str r0, [r13, #24]
    bx lr

    .global try_read_access
.thumb_func
try_read_access:
    ldr r3, .Lc.landing_pad
    ldr r2, .L1.landing_place
    str r3, [r2]
    mov r3, #1
    ldr r0, [r0]
    mov r3, #0
.L1.landing_pad:
    ldr r2, .L1.error_val
    str r3, [r2]
# Reset the landing place, so a HardFault after a test
# will stay in the loop
    mov r3, #0
    ldr r2, .L1.landing_place
    str r3, [r2]

    bx lr

.align 2
.L1.landing_place:
    .word landing_place

.Lc.landing_pad:
    .word .L1.landing_pad

.L1.error_val:
    .word error_val

.data
.globl error_val
error_val:
.word 0
.globl landing_place
landing_place:
.word 0
