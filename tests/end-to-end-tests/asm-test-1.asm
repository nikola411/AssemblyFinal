.section .data
.global num1, num2, result

num1:
    .word 5
num2:
    .word 10
result:
    .word 0

.global start

start:
    ld [%r1 + 0xFF], %r1  
    ld num2, %r2     
    add %r1, %r2     
    st %r2, result   
    halt
.end