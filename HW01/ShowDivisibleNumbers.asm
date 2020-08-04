.data
m: .asciiz "\n----- Show Divisible Numbers -----\n"
mes: .asciiz "Enter first integer: " 
mes2: .asciiz "Enter second integer: "
mes3: .asciiz "Enter divide integer: " 
mes4: .asciiz "\nResult: "
comma: .asciiz ","
dot: .asciiz "."
fin: .asciiz "\n\n"
.text

main:

la $a0, m
li $v0, 4
syscall

la $a0, mes
li $v0, 4
syscall

li $v0, 5
syscall 
move $t1, $v0

la $a0, mes2
li $v0, 4
syscall

li $v0, 5
syscall 
move $t2, $v0

la $a0, mes3
li $v0, 4
syscall

li $v0, 5
syscall 
move $t3, $v0

la $a0, mes4
li $v0, 4
syscall

	loop:
	blt $t2, $t1, exit
	div $t1, $t3
	mfhi $t4
	beq $t4, $zero, print
	next:
	addi $t1, $t1, 1
	j loop

j exit

print:
	move $a0, $t1
	li $v0, 1
	syscall

	la $a0, comma
	li $v0, 4
	syscall

	j next

exit:
	la $a0, dot
	li $v0, 4
	syscall

	la $a0, fin
	li $v0, 4
	syscall

	li $v0, 10
	syscall