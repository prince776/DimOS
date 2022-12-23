.text

.include "arch/x86_64/common.S"

.globl yield

yield: // takes a pointer to control block of kthread that called yield
    pushq %rdi // the pointer to control block of this thread (controlBlock)
    pushq $exec_continuation // want to continue back from here (rip)
    pusha64
    pushq %rsp

    movq %rsp, %rdi     // get the stack pointer as param to switch_task
    call scheduleKThread

    exec_continuation:
    // the rsp value here is already set correctly
    popa64
    add $16, %rsp
    ret

.globl switch_kthread
switch_kthread: // takes a pointer to control block of kthread that called yield
    // set rsp and rip (jmp)
    movq (%rdi), %rsp
    movq 8(%rdi), %rax 
    jmp *%rax
