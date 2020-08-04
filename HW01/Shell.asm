.data
mes: .asciiz "\n\n---------- PROCESS MENU ----------\n\t1. Show Divisible Numbers \n\t2. Binary Search \n\t3. Linear Search \n\t4. Selection Sort \n\t5. Exit"
mes2: .asciiz " \n\nEnter the Process Number (1-5) (Enter 6 for Menu): "
mes3: .asciiz "Invalid Process Number!!! Process Number can be 1,2,3,4 and 5. Try Again! \n\nEnter the Process Number (1-5) (Enter 6 for Menu): "
mes4: .asciiz "\n\n--- Shell.asm exited successfully. ---\n\n"
mes5: .asciiz "---> Process completed successfully. Return the Shell.asm \n"
p1: .asciiz "ShowDivisibleNumbers.asm"
p2: .asciiz "BinarySearch.asm"
p3: .asciiz "LinearSearch.asm"
p4: .asciiz "SelectionSort.asm"
.text

main:

la $a0, mes
li $v0, 4
syscall

process:
la $a0, mes2
li $v0, 4
syscall

cont:
li $v0, 5
syscall 
move $t0, $v0

blt $t0, 1, err
bgt $t0, 6, err
beq $t0, 5, exit
beq $t0, 6, main

beq $t0, 1, divb
beq $t0, 2, binb
beq $t0, 3, linb
beq $t0, 4, selb

cont2:
li $v0, 18
syscall

la $a0, mes5
li $v0, 4
syscall

j process

err:
la $a0, mes3
li $v0, 4
syscall

j cont

exit:
la $a0, mes4
li $v0, 4
syscall

li $v0, 10
syscall

divb:
la $a0, p1
j cont2

binb:
la $a0, p2
j cont2

linb:
la $a0, p3
j cont2

selb:
la $a0, p4
j cont2