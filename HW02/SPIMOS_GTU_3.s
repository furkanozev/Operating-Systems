.data
.text
.globl main

main:
    li $a0, 6
    li $v0, 19
    syscall

    li $a0, 3
    li $v0, 23
    syscall

    addi $t0, $v0, 1

random2:
    li $a0, 3
    li $v0, 23
    syscall
    addi $t1, $v0, 1

    beq $t0, $t1, random2

    li $t2, 0

loop:
    li $v0, 20
    syscall

    li $v0, 22
    syscall

    beq $v0, -1, finish

    move $a0, $t0
    li $v0, 21
    syscall

    addi $t2, $t2, 1
    beq $t2, 3, swap

    j loop

swap:
    move $t0, $t1
    j loop

finish:
    li $v0, 18
    syscall