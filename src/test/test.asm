.data
prompt1: .asciiz "Enter the sequence index\n"
prompt2: .asciiz "The Fibonacci value is:\n"

.text
# Print prompt1
li $v0, 4
la $a0, prompt1
syscall

# Read string
li $v0, 5
syscall
