# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
	# movl $stack_top, %esp

	# Call kernel early with grub multiboot info
	# push %eax
	# push %ebx
	# call kernel_early

	# Setup descriptor tables
	# call init_descriptor_tables

	# Call the global constructors.
	call _init

	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
1:	hlt
	jmp 1b

.size _start, . - _start
