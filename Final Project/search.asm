.data
Arr2: 
	.align 2
	.space 6000
.text

main:

la $s0, Arr2

move $t0, $zero
move $t1, $zero
move $s1, $s0

loop1:
li $v1, 1
li $v0, 20
syscall

move $t0, $v0

bgt $zero, $v1, loop1
bgt $zero, $t0, cont

sw $v0, 0($s1)

addi $s1, $s1, 4
addi $t1, $t1, 1
j loop1

cont:
lw $s3, 0($s0)

li $t0, 1

linearSearch:
beq $t0, $t1, exit

sll $t2, $t0, 2
add $t2, $t2, $s0
lw $t2, 0($t2)

beq $t2, $s3, find
addi $t0, $t0, 1
j linearSearch

find:

move $a0, $t0
li $v0, 21
syscall

j finish

exit:
li $a0, -1
li $v0, 21
syscall

finish:

li $v0, 22
syscall