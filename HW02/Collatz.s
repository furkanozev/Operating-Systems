.data
space: .asciiz " "
newline: .asciiz "\n"
colon: .asciiz ": "
.text

main:
	li $t0, 1
	li $t1, 2
	li $t2, 3

start:
	beq $t0, 25, finish
	move $t3, $t0
	j printFirst

go:
	div $t3, $t1
	mfhi $t4
	beq $t4, $zero, even

	mult $t3, $t2
	mflo $t3
	addi $t3, 1
	j print

go2:
	beq $t3, 1, go3
	j go

go3:
	addi $t0, 1
	j start

even:
	mflo $t3
	j print

print:
	move $a0, $t3
	li $v0, 1
	syscall

	la $a0, space
	li $v0, 4
	syscall

	j go2

printFirst:
	la $a0, newline
	li $v0, 4
	syscall

	move $a0, $t3
	li $v0, 1
	syscall

	la $a0, colon
	li $v0, 4
	syscall

	j go

finish:
	la $a0, newline
	li $v0, 4
	syscall

	la $a0, newline
	li $v0, 4
	syscall

	li $v0, 18
	syscall