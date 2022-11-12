.section .text
.global loadPageDirectory
loadPageDirectory:
push %ebp
movl %esp, %ebp
movl 8(%esp), %eax
movl %eax, %cr3
movl %ebp, %esp
pop %ebp
ret

.global enablePaging
enablePaging:
push %ebp
movl %esp, %ebp
movl %cr0, %eax
or $0x80000000, %eax
movl %eax, %cr0
movl %ebp, %esp
pop %ebp
ret