.data
command: .asciiz "\n> "
.text

main:

loop:
li $v1, 1

la $a0, command
li $v0, 4
syscall

li $v0, 18
syscall

bgt $zero, $v1, exit
j loop

exit:
	
li $v0, 22
syscall
