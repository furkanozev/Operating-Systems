.data
.text

main:

li $v1, 1
loop:

li $v0, 26
syscall

bgt $zero, $v1, exit
move $a0, $v0

li $v0, 25
syscall

j loop

exit:

li $v0, 22
syscall