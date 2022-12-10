.section .text
.global loadPageDirectory
loadPageDirectory:
    mov %rdi, %cr3
    ret
