.data
m: .asciiz "\n----- Binary Search -----\n"
mes: .asciiz "Enter the size of list: "
mes2: .asciiz "Enter value: "
mes3: .asciiz "Enter target value: "
mes4: .asciiz "\nTarget value is present in list.\nIndex: "
mes5: .asciiz "\nTarget value is not present in list."
mes6: .asciiz "\n!!! The list should be given in increasing order for the binary search!"
mes7: .asciiz "\n!!! List size must be positive integer!"
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

bgt $t0, $zero, checkPoint
cont2:
addi $s1, $s1, 4
addi $t0, $t0, 1
move $t6, $v0
j loop1

checkPoint:
bgt $v0, $t6, cont2

la $a0, mes6
li $v0, 4
syscall

j finish

cont:
la $a0, mes3
li $v0, 4
syscall

li $v0, 5
syscall 
move $s3, $v0

move $t0, $zero
addi $t1, $t1, -1

binarySearch:

blt $t1, $t0, exit

sub $t3, $t1, $t0
srl $t3, $t3, 1
add $t3, $t3, $t0
sll $t4, $t3, 2
add $t4, $t4, $s0
lw $t4, 0($t4)

beq $t4, $s3, find

blt $t4, $s3, right
addi $t1, $t3, -1
j binarySearch

right:
addi $t0, $t3, 1
j binarySearch

find:
la $a0, mes4
li $v0, 4
syscall

move $a0, $t3
li $v0, 1
syscall

j finish

err:
la $a0, mes7
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

li $v0, 10
syscall