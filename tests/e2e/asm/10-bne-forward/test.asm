.section text
bne %r1, %r2, target
.skip 8
target:
halt
.end
