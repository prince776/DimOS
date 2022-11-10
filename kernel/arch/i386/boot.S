# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Reserve a stack for the initial thread.
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
	movl $stack_top, %esp

	# Setup descriptor tables
	call init_descriptor_tables

	# Call the global constructors.
	call _init

	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
1:	hlt
	jmp 1b

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
.size _start, . - _start
