.section .text
.global gdt_flush
gdt_flush:
	movl +4(%esp), %eax		# Get the pointer to gdt_ptr
	lgdt (%eax) 			# Load the new GDT pointer

	movw $0x10, %ax			# put offset to data segment in ax
	movw %ax, %ds
	movw %ax, %ss
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %es
	ljmp $0x08, $gdt_flush_ret		# 0x08 is offset to code segment in gdt
gdt_flush_ret:
	ret
