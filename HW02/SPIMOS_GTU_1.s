.data
.text
.globl main

main:
    li $a0, 3
    li $v0, 19
    syscall

    li $t0, 1

loop:
    li $v0, 20
    syscall

    li $v0, 22
    syscall

    beq $v0, -1, finish

    move $a0, $t0
    li $v0, 21
    syscall

    addi $t0, 1

    j loop

finish:
    li $v0, 18
    syscall