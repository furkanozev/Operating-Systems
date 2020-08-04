.data
.text

main:

li $t0, 0
li $t1, 100

loop:
	beq $t0, $t1, exit
	li $v0, 19
	syscall

	move $a0, $v0
	li $v0, 21
	syscall

	addi $t0, $t0, 1
	j loop

exit:

li $a0, -1
li $v0, 21
syscall

li $v0, 22
syscall