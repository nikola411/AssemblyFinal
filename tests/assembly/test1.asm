.section init
boot:
    .word 0xFFFFFFFF
    jmp loop
    ld $0xDEADBEEF, %r1
    ld $0xDEADBEEF, %r2
    ld $0xBEEFDEAD, %r3
halt
.end