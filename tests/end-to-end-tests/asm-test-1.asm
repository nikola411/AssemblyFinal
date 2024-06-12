.section .data
.global num1, num2, result

num1:
    .word 5
num2:
    .word 10
result:
    .word 0

.section .text 
.global start

starts:
    ld [%r2 + 0x6FF], %r1
    not %r1    
    ld num2, %r2     
    add %r1, %r2     
    st %r2, result   
    halt
.end