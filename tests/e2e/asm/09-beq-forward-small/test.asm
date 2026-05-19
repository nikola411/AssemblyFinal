.section text
beq %r3, %r4, target
.skip 4
target:
halt
.end
