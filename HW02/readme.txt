Furkan ÖZEV
161044036

*********** HOMEWORK 2 *******************

Design:
	
	1- Kernel is run. (Each kernel is designed in accordance with the desired strategy.)
	2- Create Process Table.
	3- Create init process.
	4- Loads multiple programs into memory using fork, execve, waitpid syscalls. (I created this syscalls.)
	5- Each process have unique id.
	6- Process Table hold the necessary information about these processes in the memory.
	7- Simulator will generate interrupts, and kernel will handle according to the Round Robin scheduling.
	8- In each interrupt, previous process information is updated. the new process goes into running state.
	9- When all the process is terminated, emulator will shut down.

	!!! Descriptions of the functions I wrote and Descriptions of the syscalls are available in the comment lines in the code. !!!

	!!! My assembly extensions are not ".asm", their extensions are ".s" .
	!!!	LinearSearch.s, Collatz.s, BinarySearch.s  instead of LinearSearch.asm, Collatz.asm, BinarySearch.asm.

Process Table:

	1- I created Process Structure.
	2- This structure include:
								Process Name
								Process ID
								Parent Process ID
								Program Counter
								Process State (READY, WAITING, RUNNING)
								REGISTERS

	3- I created ProcessTable vector to keep all process structure (vector<Process>).

Robin Round Scheduling:

	1- I keep the index of the running process in a global variable.
	2- When interrupt arrives, I am looking for the next ready process in the vector.
	3- If it comes to the end of the vector and couldn't find ready process, it turns index 1 and looks at the processes up to it's index.
	4- If it finds another ready process, the newly found process runs. If not, the last running process continues to run.
	5- The finished program is deleted from the vector and the next ready process runs.
	6- If there is no other process to run except init, init runs.

Note:

	1- As requested in the homework, I implemented POSIX system calls: fork, waitpid, execve.

	2- Each kernel is designed in accordance with the desired strategy.

	3- For every timer interrupt, OS will handle the interrupt and perform round robin scheduling.

	4- Emulator will shut down only after all the programs in memory terminate.

	5- For Kernel 2 and 3, I created Generate Random Number syscall.

	6- So every time you run kernel 2 and 3, different programs can run.

Compile and Run:

	* First, replace the syscall.h and syscall.cpp files contained in the file with the files in the CPU part of your SPIM simulator.
	(Desktop/spimsimulator-code-r730/CPU)

	* Open the terminal and go to the spim directory. (cse312@ubuntu:~/Desktop/spimsimulator-code-r730/spim)

	* Then run these commands on the terminal to compile:
											-> make
											-> sudo make install

	* You should copy assembly files to a folder. (Directory of the this folder is important to run program.)

	* Open the terminal and go to the directory where the assembly files is located.

	* Then run these command on the terminal to run Kernel:
									For Kernel 1-> spim -file SPIMOS_GTU_1.s
													
									For Kernel 2-> spim -file SPIMOS_GTU_2.s
													
									For Kernel 3-> spim -file SPIMOS_GTU_3.s


Usage:
	
	-> KERNEL 1:
		In the first strategy init process will initialize Process Table,
		load 3 different programs to the memory start them
		and will enter an infinite loop until all the processes terminate.

	-> KERNEL 2:
		Second strategy is randomly (using GENERATE_RANDOM_NUMBER syscall) (get 1 random number) choosing one of the programs
		and loads it into memory 10 times (Same program 10 different processes),
		start them and will enter an infinite loop until all the processes terminate.

	-> KERNEL 3:
		Final Strategy is choosing 2 out 3 programs randomly (using GENERATE_RANDOM_NUMBER syscall) (get 2 different random number)
		and loading each program 3 times start them
		and will enter an infinite loop until all the processes terminate.

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

		Output: 1

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

		Output: 5

	Run Example 3: Must be increasing order. (This specified in hw pdf.)

		----- Binary Search -----
		Enter the size of list: 4
		Enter value: 5
		Enter value: 8
		Enter value: 1

		!!! The list should be given in increasing order for the binary search! 


Linear Search:
	
	Run Example 1:

		----- Linear Search -----
		Enter the size of list: 5
		Enter value: 19		(Index 0)
		Enter value: -6		(Index 1)
		Enter value: 8		(Index 2)
		Enter value: -2		(Index 3)
		Enter value: 12		(Index 4)
		Enter target value: -2

		Output: 3

	Run Example 2:

		----- Linear Search -----
		Enter the size of list: 6
		Enter value: 1			(Index 0)
		Enter value: 0			(Index 1)
		Enter value: -19		(Index 2)
		Enter value: 16			(Index 3)
		Enter value: 11			(Index 4)
		Enter value: -13		(Index 5)
		Enter target value: 16

		Output: 5


Collatz:

	Run Example 1:

		1: 4 2 1 
		2: 1 
		3: 10 5 16 8 4 2 1 
		4: 2 1 
		5: 16 8 4 2 1 
		6: 3 10 5 16 8 4 2 1 
		7: 22 11 34 17 52 26 13 40 20 10 5 16 8 4 2 1 
		8: 4 2 1 
		9: 28 14 7 22 11 34 17 52 26 13 40 20 10 5 16 8 4 2 1 
		10: 5 16 8 4 2 1 
		11: 34 17 52 26 13 40 20 10 5 16 8 4 2 1 
		12: 6 3 10 5 16 8 4 2 1 
		13: 40 20 10 5 16 8 4 2 1 
		14: 7 22 11 34 17 52 26 13 40 20 10 5 16 8 4 2 1 
		15: 46 23 70 35 106 53 160 80 40 20 10 5 16 8 4 2 1 
		16: 8 4 2 1 
		17: 52 26 13 40 20 10 5 16 8 4 2 1 
		18: 9 28 14 7 22 11 34 17 52 26 13 40 20 10 5 16 8 4 2 1 
		19: 58 29 88 44 22 11 34 17 52 26 13 40 20 10 5 16 8 4 2 1 
		20: 10 5 16 8 4 2 1 
		21: 64 32 16 8 4 2 1 
		22: 11 34 17 52 26 13 40 20 10 5 16 8 4 2 1 
		23: 70 35 106 53 160 80 40 20 10 5 16 8 4 2 1 
		24: 12 6 3 10 5 16 8 4 2 1 