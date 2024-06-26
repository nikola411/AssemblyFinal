.section section1
loop:
    ld num1, %r2
    st %r3, num2

.section section2
.global num1, num2
num1:
    .word 0xFF
num2:
    .word 0x8FF