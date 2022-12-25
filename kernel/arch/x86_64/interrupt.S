.text

.macro isr_no_err_stub code
    isr_stub_\code:
        pushq $0
        pushq $\code
        jmp isr_common_stub
.endm

.macro isr_err_stub code
    isr_stub_\code:
        pushq $\code
        jmp isr_common_stub
.endm

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

// For Interrupts from PIC
isr_no_err_stub 32
isr_no_err_stub 33
isr_no_err_stub 34
isr_no_err_stub 35
isr_no_err_stub 36
isr_no_err_stub 37
isr_no_err_stub 38
isr_no_err_stub 39
isr_no_err_stub 40
isr_no_err_stub 41
isr_no_err_stub 42
isr_no_err_stub 43
isr_no_err_stub 44
isr_no_err_stub 45
isr_no_err_stub 46
isr_no_err_stub 47

.globl isr_stub_table
isr_stub_table:
    .quad isr_stub_0
    .quad isr_stub_1
    .quad isr_stub_2
    .quad isr_stub_3
    .quad isr_stub_4
    .quad isr_stub_5
    .quad isr_stub_6
    .quad isr_stub_7
    .quad isr_stub_8
    .quad isr_stub_9
    .quad isr_stub_10
    .quad isr_stub_11
    .quad isr_stub_12
    .quad isr_stub_13
    .quad isr_stub_14
    .quad isr_stub_15
    .quad isr_stub_16
    .quad isr_stub_17
    .quad isr_stub_18
    .quad isr_stub_19
    .quad isr_stub_20
    .quad isr_stub_21
    .quad isr_stub_22
    .quad isr_stub_23
    .quad isr_stub_24
    .quad isr_stub_25
    .quad isr_stub_26
    .quad isr_stub_27
    .quad isr_stub_28
    .quad isr_stub_29
    .quad isr_stub_30
    .quad isr_stub_31
    // For PIC
    .quad isr_stub_32
    .quad isr_stub_33
    .quad isr_stub_34
    .quad isr_stub_35
    .quad isr_stub_36
    .quad isr_stub_37
    .quad isr_stub_38
    .quad isr_stub_39
    .quad isr_stub_40
    .quad isr_stub_41
    .quad isr_stub_42
    .quad isr_stub_43
    .quad isr_stub_44
    .quad isr_stub_45
    .quad isr_stub_46
    .quad isr_stub_47

.extern isr_handler

.include "arch/x86_64/common.S"

.globl interrupt_return_position

isr_common_stub:
    cld
    pusha64

    movq %rsp, %rdi     // get the stack pointer as param to isr_handler
    call isr_handler

interrupt_return_position:
    popa64
    add $16, %rsp

    iretq

.global idt_reload
idt_reload:
    lidt (%rdi)
    ret
