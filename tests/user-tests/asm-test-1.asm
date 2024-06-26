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
.extern undefsym
.extern jmplabl

.skip 16
sym:
    .word 0x50
sym2:
    .word sym

start:
    ld [%r1 + undefsym], %r1  
    ld num2, %r2     
    add %r1, %r2
    jmp jmplabl  
    st %r2, result   
    halt
.end