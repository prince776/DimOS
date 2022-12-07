# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
	# Call kernel early with grub multiboot info
	call kernel_early

	# Setup descriptor tables
	// call init_descriptor_tables

	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
1:	hlt
	jmp 1b

.size _start, . - _start
