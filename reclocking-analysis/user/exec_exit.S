.code64

.section .exec_exit, "ax"
cmp $0, (%r12)
jz .exec_exit
ret
