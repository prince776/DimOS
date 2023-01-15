.text

.include "arch/x86_64/common.S"

.globl return_from_interrupt
.extern interrupt_return_position
.extern signalEOI
return_from_interrupt: // takes a pointer to control block of kthread that called yield
    movq (%rdi), %rsp // Change to next kthread's stack
    
    // push a saved (or dummy if first time) ISR frame, then we can pop them and iretq
    // TODO: Don't need to build the interrupt frame if already present (i.e thread had started)
    // popq %rax // TODO: Is rsp in interrupt stack frame the rsp value before interrupt or after pushing ss
    
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

    movq $32, %rdi // set param for signalEOI
    call signalEOI // IMP: Must do, as we are not unwinding stack from ISR handlers, we are skipping that

    jmp interrupt_return_position
