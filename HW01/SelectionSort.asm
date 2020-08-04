.data
m: .asciiz "\n----- Selection Sort -----\n"
mes: .asciiz "Enter the size of list: "
mes2: .asciiz "Enter value: "
mes3: .asciiz "\nSorted List: "
mes4: .asciiz "\n!!! List size must be positive integer!\n\n"
comma: .asciiz ","
dot: .asciiz "."
fin: .asciiz "\n\n"
Arr: 
	.align 2
	.space 600
.text

main:

la $s0, Arr

la $a0, m
li $v0, 4
syscall

la $a0, mes
li $v0, 4
syscall

li $v0, 5
syscall 
move $t1, $v0

bge $zero, $t1, err

move $t0, $zero
move $s1, $s0

loop1:
beq $t0, $t1, selectionSort

la $a0, mes2
li $v0, 4
syscall

li $v0, 5
syscall 
sw $v0, 0($s1)

addi $s1, $s1, 4
addi $t0, $t0, 1
j loop1

selectionSort:
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
la $a0, mes3
li $v0, 4
syscall

move $t0, $zero

	loop4:
	beq $t0, $t1, exit

	sll $t6, $t0, 2
	add $t6, $t6, $s0
	lw $s6, 0($t6)

	move $a0, $s6
	li $v0, 1
	syscall

	la $a0, comma
	li $v0, 4
	syscall

	addi $t0, $t0, 1
	j loop4

exit:

la $a0, dot
li $v0, 4
syscall

la $a0, fin
li $v0, 4
syscall

li $v0, 10
syscall

err:
la $a0, mes4
li $v0, 4
syscall

li $v0, 10
syscall