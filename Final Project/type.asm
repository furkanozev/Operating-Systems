.data
m: .asciiz "-1"
.text

main:

li $v1, 1
loop:

li $v0, 23
syscall

bgt $zero, $v1, exit
move $a0, $v0

li $v0, 24
syscall

j loop

exit:

la $a0, m
li $v0, 25
syscall

li $v0, 22
syscall