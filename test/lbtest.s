.data
test: .word 0xFFFF
.text
lb $t0, test($t1)
