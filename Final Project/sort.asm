.data
Arr: 
	.align 2
	.space 6000
.text

main:

la $s0, Arr

move $t0, $zero
move $t1, $zero
move $s1, $s0

loop1:
li $v1, 1
li $v0, 20
syscall
move $t0, $v0

bgt $zero, $v1, loop1
bgt $zero, $t0, selectionSort

sw $v0, 0($s1)

addi $s1, $s1, 4
addi $t1, $t1, 1
j loop1

selectionSort:
beq $t1, $zero, exit
move $t0, $zero
addi $t2, $t1, -1
	
	loop2:
	bge $t0, $t2, print
	move $t3, $t0
	addi $t4, $t0, 1	
		loop3:
		bge $t4, $t1, cont

		sll $t5, $t4, 2
		add $t5, $t5, $s0
		lw $s5, 0($t5)

		sll $t6, $t3, 2
		add $t6, $t6, $s0
		lw $s6, 0($t6)

		bge $s5, $s6, ifb
		move $t3, $t4

		ifb:
		addi $t4, $t4, 1
		j loop3

	cont:
	sll $t5, $t0, 2
	add $t5, $t5, $s0
	lw $s5, 0($t5)

	sll $t6, $t3, 2
	add $t6, $t6, $s0
	lw $s6, 0($t6)

	sw $s6, 0($t5)
	sw $s5, 0($t6)

	addi $t0, $t0, 1
	j loop2

print:
move $t0, $zero

	loop4:
	beq $t0, $t1, exit

	sll $t6, $t0, 2
	add $t6, $t6, $s0
	lw $s6, 0($t6)

	move $a0, $s6
	li $v0, 21
	syscall

	addi $t0, $t0, 1
	j loop4

exit:

li $v0, 22
syscall