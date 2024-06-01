.section init
    ld $10, %r1           ; Load the value 10 into register r1
    ld $20, %r2           ; Load the value 20 into register r2
    add %r1, %r2          ; Add the values in r1 and r2, store the result in r2

    ld $25, %r3           ; Load the value 25 into register r3
    bgt %r2, %r3, greater ; If r2 > r3, branch to the label 'greater'

    ; If r2 is not greater than r3
    ld $0, %r0            ; Load the value 0 into register r0
    jmp end               ; Jump to the end of the program

greater:
    ; If r2 is greater than r3
    ld $1, %r0            ; Load the value 1 into register r0

end:
    halt                  ; Halt the execution
.end