Furkan ÖZEV
161044036

*********** HOMEWORK 1 *******************

Design:
	
	1- Shell program is run.
	2- The user enters the process to be run.
	3- CreateProcess system procedure are called. (I created this process.)
	4- Loads assembly file of this process from the disk and executes it.
	5- After this call is processed, the OS returns to the caller (Shell).
	6- Shell resumes working.

Note:

	1- As requested in the homework, shell.asm is not run again, it continues from where it left off.

	2- Before running the new process, variables related to memory and register are backed up.

	3- After running the Initial World function, I loaded the assembly file to be run.

	4- The new process runs and finishes.

	5- Backup variables are restored.

	6- I designed a menu to make operations easier so you won't have to worry about entering the file name.

Compile and Run:

	* First, replace the syscall.h and syscall.cpp files contained in the file with the files in the CPU part of your SPIM simulator.
	(Desktop/spimsimulator-code-r730/spim)

	* Open the terminal and go to the directory where the files (syscall.h and syscall.cpp) is located. (cse312@ubuntu:~/Desktop/spimsimulator-code-r730/spim)

	* Then run these commands on the terminal to compile:
											-> make
											-> sudo make install

	* You should copy assembly files to a folder. (Directory of the this folder is important to run program.)

	* Open the terminal and go to the directory where the assembly files (Shell.asm and others) is located.

	* Then run these command on the terminal to run Shell:
											-> spim read Shell.asm
													OR
											-> spim file Shell.asm

Usage:
	
	*** I designed a menu to make operations easier so you won't have to worry about entering the file name.

	* First, a menu will welcome you.

		---------- PROCESS MENU ----------
		1. Show Divisible Numbers	(Enter 1 for this process)
		2. Binary Search			(Enter 2 for this process)
		3. Linear Search 			(Enter 3 for this process)
		4. Selection Sort 			(Enter 4 for this process)
		5. Exit 					(Enter 5 for exit)

	* Then enter the process number to run process.

		Enter the Process Number (1-5) (Enter 6 for Menu):

	* You can enter 6 to show menu again.

	* For entries other than these, you will receive an error message and you will be asked to enter again.

		Enter the Process Number (1-5) (Enter 6 for Menu): 7
		Invalid Process Number!!! Process Number can be 1,2,3,4 and 5. Try Again! 

		Enter the Process Number (1-5) (Enter 6 for Menu): -3
		Invalid Process Number!!! Process Number can be 1,2,3,4 and 5. Try Again!

Show Divisible Numbers:
	
	Run Example 1:

		Enter the Process Number (1-5) (Enter 6 for Menu): 1

		----- Show Divisible Numbers -----
		Enter first integer: 10
		Enter second integer: 20
		Enter divide integer: 3

		Result: 12,15,18,.

		---> Process completed successfully. Return the Shell.asm 

	Run Example 2:

		Enter the Process Number (1-5) (Enter 6 for Menu): 1

		----- Show Divisible Numbers -----
		Enter first integer: -25
		Enter second integer: 17
		Enter divide integer: 7

		Result: -21,-14,-7,0,7,14,.

		---> Process completed successfully. Return the Shell.asm 

Binary Search:
	
	Run Example 1:

		----- Binary Search -----
		Enter the size of list: 5
		Enter value: -6		(Index 0)
		Enter value: -2		(Index 1)
		Enter value: 8		(Index 2)
		Enter value: 12		(Index 3)
		Enter value: 19		(Index 4)
		Enter target value: -2

		Target value is present in list.
		Index: 1

		---> Process completed successfully. Return the Shell.asm 

	Run Example 2:

		----- Binary Search -----
		Enter the size of list: 6
		Enter value: -19		(Index 0)
		Enter value: -3			(Index 1)
		Enter value: 0			(Index 2)
		Enter value: 1			(Index 3)
		Enter value: 11			(Index 4)
		Enter value: 16			(Index 5)
		Enter target value: 16

		Target value is present in list.
		Index: 5

		---> Process completed successfully. Return the Shell.asm 

	Run Example 3: Must be increasing order. (This specified in hw pdf.)

		Enter the Process Number (1-5) (Enter 6 for Menu): 2

		----- Binary Search -----
		Enter the size of list: 4
		Enter value: 5
		Enter value: 8
		Enter value: 1

		!!! The list should be given in increasing order for the binary search!

		---> Process completed successfully. Return the Shell.asm 

Linear Search:
	
	Run Example 1:

		Enter the Process Number (1-5) (Enter 6 for Menu): 3

		----- Linear Search -----
		Enter the size of list: 5
		Enter value: 19		(Index 0)
		Enter value: -6		(Index 1)
		Enter value: 8		(Index 2)
		Enter value: -2		(Index 3)
		Enter value: 12		(Index 4)
		Enter target value: -2

		Target value is present in list.
		Index: 3

		---> Process completed successfully. Return the Shell.asm

	Run Example 2:

		Enter the Process Number (1-5) (Enter 6 for Menu): 3

		----- Linear Search -----
		Enter the size of list: 6
		Enter value: 1			(Index 0)
		Enter value: 0			(Index 1)
		Enter value: -19		(Index 2)
		Enter value: 16			(Index 3)
		Enter value: 11			(Index 4)
		Enter value: -13		(Index 5)
		Enter target value: 16

		Target value is present in list.
		Index: 5

		---> Process completed successfully. Return the Shell.asm 

Selection Sort:

	Run Example 1:

		Enter the Process Number (1-5) (Enter 6 for Menu): 4

		----- Selection Sort -----
		Enter the size of list: 5
		Enter value: 19
		Enter value: -6
		Enter value: 8
		Enter value: -2
		Enter value: 12

		Sorted List: -6,-2,8,12,19,.

		---> Process completed successfully. Return the Shell.asm 

	Run Example 2:

		Enter the Process Number (1-5) (Enter 6 for Menu): 4

		----- Selection Sort -----
		Enter the size of list: 6
		Enter value: 1
		Enter value: 0
		Enter value: -19
		Enter value: 16
		Enter value: 11
		Enter value: -13

		Sorted List: -19,-13,0,1,11,16,.

		---> Process completed successfully. Return the Shell.asm 

Show Menu:

	Run Example:

		Enter the Process Number (1-5) (Enter 6 for Menu): 6

		---------- PROCESS MENU ----------
			1. Show Divisible Numbers 
			2. Binary Search 
			3. Linear Search 
			4. Selection Sort 
			5. Exit

Exit:
	
	Run Example:

		Enter the Process Number (1-5) (Enter 6 for Menu): 5

		--- Shell.asm exited successfully. ---
