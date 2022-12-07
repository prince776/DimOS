.text

.macro isr_no_err_stub code
    isr_stub_\code:
        pushq $0
        pushq $\code
        jmp isr_common_stub
.endm

.macro isr_err_stub code
    isr_stub_\code:
        push %rbp
        movq %rsp, %rbp
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

.macro pushagrd
push %rax
push %rbx
push %rcx
push %rdx
push %rsi
push %rdi
.endm

.macro popagrd
pop %rdi
pop %rsi
pop %rdx
pop %rcx
pop %rbx
pop %rax
.endm

.macro pushacrd
movq %cr0, %rax
push %rax
movq %cr2, %rax
push %rax
movq %cr3, %rax
push %rax
movq %cr4, %rax
push %rax
.endm

.macro popacrd
pop %rax
movq %rax, %cr4
pop %rax
movq %rax, %cr3
pop %rax
movq %rax, %cr2
pop %rax
movq %rax, %cr0
.endm

.extern isr_handler

isr_common_stub:
    push %rbp
    movq %rsp, %rbp
    pushagrd
    pushacrd
    movw %ds, %ax
    pushq %rax
    pushq $0
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    leaq 0x10(%rsp), %rdi
    call isr_handler

    pop %rax
    pop %rax
    movw %ax, %ds
    movw %ax, %es
    popacrd
    popagrd
    pop %rbp
    add $0x10, %rsp
    iretq

.global idt_reload
idt_reload:
    push %rbp
    movq %rsp, %rbp
    pushfq
    cli
    lidt (%rdi)
    popfq
    pop %rbp
    ret
