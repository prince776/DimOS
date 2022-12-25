.text

.include "arch/x86_64/common.S"

.globl yield

yield: // takes a pointer to control block of kthread that called yield
    pushq %rdi // the pointer to control block of this thread (controlBlock)
    pushq $exec_continuation // want to continue back from here (rip)
    pusha64
    pushq %rsp

    movq %rsp, %rdi     // get the stack pointer as param to scheduleKThread
    call scheduleKThread

    exec_continuation:
    // the rsp value here is already set correctly
    popa64
    add $16, %rsp
    ret

.globl return_from_yield
return_from_yield: // takes a pointer to control block of kthread that called yield
    // set rsp and rip (jmp)
    movq (%rdi), %rsp // Change to next kthread's stack
    movq 8(%rdi), %rax 
    jmp *%rax


.globl return_from_interrupt
.extern interrupt_return_position
.extern signalEOI
return_from_interrupt: // takes a pointer to control block of kthread that called yield
    movq (%rdi), %rsp // Change to next kthread's stack
    
    // push a saved (or dummy if first time) ISR frame, then we can pop them and iretq

    // Interrupt Frame
    movq %ss, %rax // ss
    pushq %rax

    pushq (%rdi) // rsp

    pushq 136(%rdi) // rflags

    movq %cs, %rax // cs
    pushq %rax

    pushq 8(%rdi) // rip

    movq $0, %rax
    pushq %rax // errorCode
    pushq %rax // isrNum

    // Gen registers
    pushq 128(%rdi)
    pushq 120(%rdi)
    pushq 112(%rdi)
    pushq 104(%rdi)
    pushq 96(%rdi)
    pushq 88(%rdi)
    pushq 80(%rdi)
    pushq 72(%rdi)
    pushq 64(%rdi)
    pushq 56(%rdi)
    pushq 48(%rdi)
    pushq 40(%rdi)
    pushq 32(%rdi)
    pushq 24(%rdi)
    pushq 16(%rdi)

    movq $32, %rdi
    call signalEOI // IMP: Must do, as we are not unwinding stack from ISR handlers, we are skipping that

    jmp interrupt_return_position
