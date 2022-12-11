.text
.globl gdt_flush

gdt_flush:
  lgdt (%rdi)

  movw $0x30, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  movw %ax, %ss

  pop %rdi

  movq $0x28, %rax
  push %rax
  push %rdi
  retfq
