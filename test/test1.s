# SOL COMMENT
.data


.text
A:    # EOL COMMENT
B: syscall
C: E: .asciiz "Test: FFFF: something"
D: addiu $t0, $t1, 0x40
add $t0, $t1, $t0
addiu $t1, $t2, 0x50
lw $t1, 0x40($t0)
li $t4, %lo(A)
