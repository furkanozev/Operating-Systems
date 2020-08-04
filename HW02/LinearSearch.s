.data
m: .asciiz "\n----- Linear Search -----\n"
mes: .asciiz "Enter the size of list: "
mes2: .asciiz "Enter value: "
mes3: .asciiz "Enter target value: "
mes4: .asciiz "\nOutput: "
mes5: .asciiz "\nOutput: -1"
mes6: .asciiz "\n!!! List size must be positive integer!"
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
	beq $t0, $t1, cont

	la $a0, mes2
	li $v0, 4
	syscall

	li $v0, 5
	syscall 
	sw $v0, 0($s1)

	addi $s1, $s1, 4
	addi $t0, $t0, 1
	j loop1

cont:
	la $a0, mes3
	li $v0, 4
	syscall

	li $v0, 5
	syscall 
	move $s3, $v0

	move $t0, $zero
	addi $t1, $t1, -1

linearSearch:
	bgt $t0, $t1, exit

	sll $t2, $t0, 2
	add $t2, $t2, $s0
	lw $t2, 0($t2)

	beq $t2, $s3, find
	addi $t0, $t0, 1
	j linearSearch

find:
	la $a0, mes4
	li $v0, 4
	syscall

	move $a0, $t0
	li $v0, 1
	syscall

	j finish

err:
	la $a0, mes6
	li $v0, 4
	syscall
	j finish

exit:
	la $a0, mes5
	li $v0, 4
	syscall

finish:
	la $a0, fin
	li $v0, 4
	syscall

	li $v0, 18
	syscall