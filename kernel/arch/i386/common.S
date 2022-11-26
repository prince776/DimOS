.global outb
outb:
    movb 8(%esp), %al
    movw 4(%esp) ,%dx
    out %al, %dx    
    ret         