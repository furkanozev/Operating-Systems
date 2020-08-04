/* SPIM S20 MIPS simulator.
   Execute SPIM syscalls, both in simulator and bare mode.
   Execute MIPS syscalls in bare mode, when running on MIPS systems.
   Copyright (c) 1990-2010, James R. Larus.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of the James R. Larus nor the names of its contributors may be
   used to endorse or promote products derived from this software without specific
   prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef _WIN32
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef _WIN32
#include <io.h>
#endif

#include "spim.h"
#include "string-stream.h"
#include "inst.h"
#include "reg.h"
#include "mem.h"
#include "sym-tbl.h"
#include "syscall.h"
#include "spim-utils.h"

#include <vector>
#include <string.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <errno.h>
#include <dirent.h>

using namespace std;

enum ProcessState {
  READY,
  WAITING,
  RUNNING
};

struct Process{
  char processName[1024];
  char input[1024];
  char output[1024];
  char parameter[1024];
  int fin;
  int fout;
  int fparam;
  int ProcessID;
  int run;
  int spipe;
  int background;
  mem_addr ProgramCounter;
  ProcessState state;
  reg_word registers[R_LENGTH];
  struct dirent **namelist;
  int n, n2, n3;
};

int currentProcess = 0;
int totalProcess = 0;
int pipecount = 0;
int checkflag = 0;
int checkflag2 = 0;
int generate = 0;
int finflag = 0;
int cantype = 0;
int background = 0;
int breakc = 0;

mem_addr kernelFinish;
mem_addr processFinish;

vector <Process> ProcessTable;
Process shellProc;

void findID(Process* p1, char* buf);
void swapProcess();
void printx();
void updateTable(int index);
void Restore(int index);
void freeMemory();

#ifdef _WIN32
/* Windows has an handler that is invoked when an invalid argument is passed to a system
   call. https://msdn.microsoft.com/en-us/library/a9yf33zb(v=vs.110).aspx

   All good, except that the handler tries to invoke Watson and then kill spim with an exception.

   Override the handler to just report an error.
*/

#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>

void myInvalidParameterHandler(const wchar_t* expression,
   const wchar_t* function, 
   const wchar_t* file, 
   unsigned int line, 
   uintptr_t pReserved)
{
  if (function != NULL)
    {
      run_error ("Bad parameter to system call: %s\n", function);
    }
  else
    {
      run_error ("Bad parameter to system call\n");
    }
}

static _invalid_parameter_handler oldHandler;

void windowsParameterHandlingControl(int flag )
{
  static _invalid_parameter_handler oldHandler;
  static _invalid_parameter_handler newHandler = myInvalidParameterHandler;

  if (flag == 0)
    {
      oldHandler = _set_invalid_parameter_handler(newHandler);
      _CrtSetReportMode(_CRT_ASSERT, 0); // Disable the message box for assertions.
    }
  else
    {
      newHandler = _set_invalid_parameter_handler(oldHandler);
      _CrtSetReportMode(_CRT_ASSERT, 1);  // Enable the message box for assertions.
    }
}
#endif


/* Decides which syscall to execute or simulate.  Returns zero upon
   exit syscall and non-zero to continue execution. */

int
do_syscall ()
{
#ifdef _WIN32
    windowsParameterHandlingControl(0);
#endif

  /* Syscalls for the source-language version of SPIM.  These are easier to
     use than the real syscall and are portable to non-MIPS operating
     systems. */

  switch (R[REG_V0])
    {
    case PRINT_INT_SYSCALL:
      write_output (console_out, "%d", R[REG_A0]);
      break;

    case PRINT_FLOAT_SYSCALL:
      {
	float val = FPR_S (REG_FA0);

	write_output (console_out, "%.8f", val);
	break;
      }

    case PRINT_DOUBLE_SYSCALL:
      write_output (console_out, "%.18g", FPR[REG_FA0 / 2]);
      break;

    case PRINT_STRING_SYSCALL:
      write_output (console_out, "%s", mem_reference (R[REG_A0]));
      break;

    case READ_INT_SYSCALL:
      {
	static char str [256];

	read_input (str, 256);
	R[REG_RES] = atol (str);
	break;
      }

    case READ_FLOAT_SYSCALL:
      {
	static char str [256];

	read_input (str, 256);
	FPR_S (REG_FRES) = (float) atof (str);
	break;
      }

    case READ_DOUBLE_SYSCALL:
      {
	static char str [256];

	read_input (str, 256);
	FPR [REG_FRES] = atof (str);
	break;
      }

    case READ_STRING_SYSCALL:
      {
	read_input ( (char *) mem_reference (R[REG_A0]), R[REG_A1]);
	data_modified = true;
	break;
      }

    case SBRK_SYSCALL:
      {
	mem_addr x = data_top;
	expand_data (R[REG_A0]);
	R[REG_RES] = x;
	data_modified = true;
	break;
      }

    case PRINT_CHARACTER_SYSCALL:
      write_output (console_out, "%c", R[REG_A0]);
      break;

    case READ_CHARACTER_SYSCALL:
      {
	static char str [2];

	read_input (str, 2);
	if (*str == '\0') *str = '\n';      /* makes xspim = spim */
	R[REG_RES] = (long) str[0];
	break;
      }

    case EXIT_SYSCALL:
      spim_return_value = 0;
      return (0);

    case EXIT2_SYSCALL:
      spim_return_value = R[REG_A0];	/* value passed to spim's exit() call */
      return (0);

    case OPEN_SYSCALL:
      {
#ifdef _WIN32
        R[REG_RES] = _open((char*)mem_reference (R[REG_A0]), R[REG_A1], R[REG_A2]);
#else
	R[REG_RES] = open((char*)mem_reference (R[REG_A0]), R[REG_A1], R[REG_A2]);
#endif
	break;
      }

    case READ_SYSCALL:
      {
	/* Test if address is valid */
	(void)mem_reference (R[REG_A1] + R[REG_A2] - 1);
#ifdef _WIN32
	R[REG_RES] = _read(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#else
	R[REG_RES] = read(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#endif
	data_modified = true;
	break;
      }

    case WRITE_SYSCALL:
      {
	/* Test if address is valid */
	(void)mem_reference (R[REG_A1] + R[REG_A2] - 1);
#ifdef _WIN32
	R[REG_RES] = _write(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#else
	R[REG_RES] = write(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#endif
	break;
      }

    case CLOSE_SYSCALL:
      {
#ifdef _WIN32
	R[REG_RES] = _close(R[REG_A0]);
#else
	R[REG_RES] = close(R[REG_A0]);
#endif
	break;
      }

    // This syscall is used by the shell.asm .
    // A command is received from the user and parsed in this function.
    // While the command is being parsed, the processes(like sort, search etc.) and information of the processes(like input, output, pipe etc.) are determined.
    // Processes are added to the process table with their information.
    // Then, swapProcess () function is called to run the processes.
    case READ_COMMAND_SYSCALL:
      {
        char bufferm[1025];
        char buf[1025];
        read_input (bufferm, 1024);
        char last = ' ';

        // Check command is exit, If the command is exit, the shell exits the loop.
        if(strlen(bufferm) >= 4 && strncmp(bufferm, "exit", 4) == 0)
        {
          R[3] = -1;
          break;
        }

        // The information of the shell process is kept. Because after the command(processes) is run,
        // it will be returned to the shell process.
        strcpy(shellProc.processName, "shell");
        strcpy(shellProc.input, "screen");
        strcpy(shellProc.output, "screen");
        shellProc.ProcessID = 0;
        shellProc.run = 1;
        shellProc.spipe = 0;
        shellProc.ProgramCounter = PC;
        shellProc.state = READY;
        memcpy(&shellProc.registers, &R, sizeof(R));
        if(checkflag2 == 0)
        {
          kernelFinish = current_text_pc();
          checkflag2 = 1;
        }
        int i = 0, j = 0, t = 0, flag = 0;
        unsigned int z = 0;
        currentProcess = -1;
        totalProcess = 0;

        // Parse command. Determine processes and their informations.
        while(i < (int)strlen(bufferm) && bufferm[i] != '\0')
        {
          for(j = 0; bufferm[i] != '\0'; i++)
          {
            if(bufferm[i] == '<' || bufferm[i] == '>' || bufferm[i] == '|' || bufferm[i] == '&')
              break;
            // Check process is type. Because type process take an parameter.
            if(strlen(bufferm) > 4 && i >= 4 && bufferm[i-4] == 't' && bufferm[i-3] == 'y' && bufferm[i-2] == 'p' && bufferm[i-1] == 'e' && bufferm[i] == ' ')
            {
              cantype = 1;
              buf[j] = bufferm[i];
              j += 1; 
            }
            else if(bufferm[i] != ' ' && bufferm[i] != '\n')
            {
              buf[j] = bufferm[i];
              j += 1; 
            }
          }
          buf[j] = '\0';

          if(strlen(buf) == 0)
          {
            // Pipe and background symbols cannot be repeated.
            // Because the background process does not pipe with the command.
            // I made it based on the terminal.
            // "ls&|ls" It is not possible.
            if(last == '&' && bufferm[i] == '|')
            {
              write_output (console_out, "--- syntax error near unexpected token \'|\' ---\n");
              ProcessTable.clear();
              breakc = 1;
              break;
            }

            if(bufferm[i] == '<')
            flag = 1;
            else if(bufferm[i] == '>')
              flag = 2;
            else if(bufferm[i] == '|')
              flag = 3;
            else if(bufferm[i] == '&')
              flag = 0;
            last = bufferm[i];
            i += 1;
            continue;
          }

          if(flag == 0 || flag == 3)
          {
            Process p1;
            // Determine background process.
            if(bufferm[i] == '&')
            {
              p1.background = 1;
            }
            else
              p1.background = 0;
         
            // If processes is type, then check parameter.
            if(cantype == 1)
            {
              sprintf(p1.processName, "type.asm");
              for(t = 0, z = 5; z < strlen(buf); z++, t++)
                p1.parameter[t] = buf[z];
              p1.parameter[t] = '\0';
              cantype = 0;
              if(strlen(buf) > 5)
              {
                p1.ProcessID = 4;
                p1.fparam = open(p1.parameter, O_RDWR, 0666);
                if(p1.fparam < 0)
                {
                  write_output (console_out, "--- %s: No such file or directory ---\n", p1.parameter);
                  ProcessTable.clear();
                  breakc = 1;
                  break;
                }
              }
              else
                p1.ProcessID = -1;
            }
            else
            {
              sprintf(p1.processName, "%s.asm", buf);
              // Find an id of processes according to their name. It is not process id.
              findID(&p1, buf);
            }
            if(p1.ProcessID != -1)
            {
              currentProcess += 1;
              totalProcess += 1;
              // Determines Stdin and Stdout.
              // The default version of Stdin and Stdout is screen.
              // If there is a pipe, a special file is determined for the processes with pipe.
              // If an input or output file is given as an argument, Stdin and Stdout are these files.
              if(flag == 0)
              {
                strcpy(p1.input, "screen");
                strcpy(p1.output, "screen");
                p1.spipe = 0;
              }
              // If there is a pipe, a special file is determined for the processes with pipe.
              // Stdout of the previous process and Stdin of the current process will be the same file.
              // This file is determined specially for pipe operations.
              else if(ProcessTable[currentProcess - 1].background == 0)
              {
                p1.spipe = 1;
                char tempbuf[1024];
                pipecount += 1;
                sprintf(tempbuf, "pipecount%d.txt", pipecount);
                strcpy(ProcessTable[currentProcess - 1].output, tempbuf);
                strcpy(p1.input, tempbuf);

                ProcessTable[currentProcess - 1].fout = open(tempbuf, O_RDWR | O_CREAT | O_TRUNC, 0666);
                p1.fin = open(tempbuf, O_RDWR, 0666);
              }
              // Scans the directory for "ls" process.
              if(p1.ProcessID == 5)
              {
                p1.n = scandir(".", &(p1.namelist), NULL, alphasort);
                p1.n2 = -1;
                p1.n3 = 0;
              }
              // Determine process is background.
              if(p1.background == 1)
                background = 1;
              // The information of the process is filled in and added to the process table.
              p1.state = READY;
              p1.run = 0;
              p1.ProgramCounter = current_text_pc();
              // Load assembly file using read_assembly_file function.
              initialize_symbol_table();
              if(read_assembly_file(p1.processName) == 0)
                return -1;
              memcpy(&p1.registers, &R, sizeof(R));
              ProcessTable.push_back(p1);
              processFinish = current_text_pc();
            }
            else
            {
              // If no command is found, it prints an error message on the screen.
              write_output (console_out, "--- No command \'%s\' found ---\n", buf);
              ProcessTable.clear();
              breakc = 1;
              break;
            }
          }
          // If an input file is given as an argument, Stdin will be this file.
          // If the process background is a task, the input file given later does not change the stdin.
          // I made it based on the terminal.
          // Like in this command "sort& < 1.txt", It will take the input on the screen.
          // The correct use should be as follows: "sort < 1.txt &". Stdin will be "1.txt"
          else if(flag == 1 && ProcessTable[currentProcess].background == 0)
          {
            ProcessTable[currentProcess].spipe = 0;
            strcpy(ProcessTable[currentProcess].input, buf);
            ProcessTable[currentProcess].fin = open(buf, O_RDWR, 0666);
            if(ProcessTable[currentProcess].fin < 0)
            {
              printf("--- Error: Input file(%s) doesn't found ---\n", buf);
              ProcessTable.clear();
              breakc = 1;
              break;
            }
          }
          // If an output file is given as an argument, Stdout will be this file.
          // If the process background is a task, the output file given later does not change the stdout.
          // I made it based on the terminal.
          // Like in this command "ls& > 1.txt", It will print the output on the screen.
          // The correct use should be as follows: "ls > 1.txt &". Stdout will be "1.txt". 
          else if(flag == 2 && ProcessTable[currentProcess].background == 0)
          {
            strcpy(ProcessTable[currentProcess].output, buf);
            ProcessTable[currentProcess].fout = open(buf, O_RDWR | O_CREAT | O_TRUNC, 0666);
          }

          if(bufferm[i] == '<')
            flag = 1;
          else if(bufferm[i] == '>')
            flag = 2;
          else if(bufferm[i] == '|')
            flag = 3;
          else if(bufferm[i] == '&')
            flag = 0;
          last = bufferm[i];

          if(currentProcess >= 0 && bufferm[i] == '&')
          {
            ProcessTable[currentProcess].background = 1;
            background = 1;
          }

          i += 1;
        }

        if(flag != 0 && breakc == 0 && strlen(buf) == 0)
        {
          write_output (console_out, "--- syntax error near unexpected token ---\n");
          ProcessTable.clear();
          breakc = 1;
        }

        currentProcess = 0;
        write_output(console_out, "\n");
        // Process table is filled. If there is no error, the swapProcess () function is now called to run the processes.
        swapProcess();
        break;
      }

  // This syscall is used by the random.asm .
  // It generates a random number and writes it to R [V0].
  // Then, swapProcess () function is called to run next the processes for multi programming or parallel running.
  case GENERATE_RANDOM_NUMBER:
  {
    if(generate == 0)
    {
      srand (time(NULL));
      generate = 1;
    }
    int number;
    number = rand() % 99 + 1;

    R[REG_V0] = number;
    swapProcess();
    break;
  }

  // This syscall is used by the sort.asm and search.asm .
  // Reads 1 integer from Stdin and writes it to R [V0].
  // Then, swapProcess () function is called to run next the processes for multi programming or parallel running.
  case READ_INT_INPUT:
  {
    // If Stdin is the screen, it takes the numbers from the user.
    if(strcmp(ProcessTable[currentProcess].input, "screen") == 0)
    {
      write_output (console_out, "%s", "Enter number: ");
      static char str [256];
      read_input (str, 256);
      R[REG_V0] = atol (str);
    }
    // Otherwise, reads 1 integer from the file.
    else
    {
      char temp = ' ';
      char tempbuf[1024];
      int bytesread = 0;
      int i = 0, flag = 0, number = 0;
      do
      {
        while(((bytesread = read(ProcessTable[currentProcess].fin, &temp, 1)) == -1) && (errno == EINTR));
        if(bytesread <= 0)
        {
          flag = -1;
          break;
        }
        i += 1;
      }while(temp != ' ');

      if(i > 0)
        flag = 1;

      lseek(ProcessTable[currentProcess].fin, i*-1, SEEK_CUR);

      if(flag == -1)
      {
        // If it is a process pipe, input may come.
        // Check their pipe process partner is finished.
        if(ProcessTable[currentProcess].spipe == 1)
        {
          ProcessTable[currentProcess].state = WAITING;
          R[3] = -1;
        }
        else
        {
          // If there are no more integer, it returns -1.
          R[REG_V0] = -1;
        }
      }
      else
      {
        while(((bytesread = read(ProcessTable[currentProcess].fin, tempbuf, i)) == -1) && (errno == EINTR));
        tempbuf[i] = '\0';
        number = atol(tempbuf);
        R[REG_V0] = number;
      }
    }

    swapProcess();
    break;
  }

  // This syscall is used by the random.asm, sort.asm and search.asm .
  // Prints 1 integer to Stdout.
  // Then, swapProcess () function is called to run next the processes for multi programming or parallel running.
  case PRINT_INT_OUTPUT:
  {
    // If Stdout is the screen, it prints the numbers to the screen.
    if(strcmp(ProcessTable[currentProcess].output, "screen") == 0)
    {
      write_output(console_out, "%d ", R[REG_A0]);
    }
    // Otherwise, prints 1 integer to file.
    else
    {
      char printbuf[1024];
      int bytesread = 0, byteswritten = 0;
      char* bp;

      sprintf(printbuf, "%d ", R[REG_A0]);

      bytesread = strlen(printbuf);
      bp = printbuf;
      while(bytesread > 0)
      {
        while(((byteswritten = write(ProcessTable[currentProcess].fout, bp, bytesread)) == -1) && (errno = EINTR));
        if(byteswritten < 0)
          break;
        bytesread -= byteswritten;
        bp += byteswritten;
      }

    }
    swapProcess();
    break;
  }

  // All assembly files use this syscall to exit.
  // Deletes the process that finished from the process table.
  // Then, swapProcess () function is called to run next the processes for multi programming or parallel running.
  case PROCESS_EXIT:
  {
    if(ProcessTable.size() == 0)
    {
      write_output(console_out, "\n");
      spim_return_value = 0;
      // If all processes are finished, it will free memory.
      freeMemory();
      return (0);
    }
    else
    {
      if(ProcessTable[currentProcess].background == 1)
        write_output(console_out, "\n\n");
      else if(ProcessTable.size() == 1)
        write_output(console_out, "\n");

      if(ProcessTable.size() > 1 && currentProcess + 1 < totalProcess)
        ProcessTable[currentProcess+1].spipe = -1;

      // Close stdin and stdout.
      if(strcmp(ProcessTable[currentProcess].input, "screen") != 0)
        close(ProcessTable[currentProcess].fin);
      if(strcmp(ProcessTable[currentProcess].output, "screen") != 0)
        close(ProcessTable[currentProcess].fout);
      ProcessTable.erase(ProcessTable.begin() + currentProcess);
      currentProcess -= 1;
      totalProcess -= 1;

      // If all processes are finished, it will deletes pipe files.
      if(ProcessTable.size() == 0)
      {
        char tempbuf[1024];
        for(; pipecount > 0; pipecount--)
        {
          sprintf(tempbuf, "pipecount%d.txt", pipecount);
          unlink(tempbuf);
        }
        ProcessTable.clear();
      }

      finflag = 1;

      swapProcess();
    }
    break;
  }

  // This syscall is used by the type.asm .
  // This is special for syscall type process. Because type process takes a file as a parameter.
  // Reads 1 character to file parameter.
  // Then, swapProcess () function is called to run next the processes for multi programming or parallel running.
  case READ_CHAR_INPUT:
  {
    int bytesread = 0;
    char ch;
    bytesread = read(ProcessTable[currentProcess].fparam, &ch, 1);
    // If there are no more character in file , it returns -1.
    if(bytesread <= 0)
    {
      R[3] = -1;
      break;
    }

    static char str [2];

    str[0] = ch;
    str[1] = '\0';
    R[REG_RES] = (long) str[0];
    R[3] = 1;

    swapProcess();
    break;
  }

  // This syscall is used by the type.asm .
  // Prints 1 character to Stdout.
  // Then, swapProcess () function is called to run next the processes for multi programming or parallel running.
  case PRINT_CHAR_OUTPUT:
  {
    // If Stdout is the screen, it prints the character to the screen.
    if(strcmp(ProcessTable[currentProcess].output, "screen") == 0)
    {
      write_output(console_out, "%c", R[REG_A0]);
    }
    // Otherwise, print 1 character to file.
    else
    {
      int byteswritten = 0;
      char ch = R[REG_A0];
      byteswritten = write(ProcessTable[currentProcess].fout, &ch, 1);
      if(byteswritten <= 0)
      {
        R[3] = -1;
        break;
      }
    }

    swapProcess();
    break;
  }

  // This syscall is used by the type.asm and ls.asm .
  // Prints the string to Stdout.
  // Then, swapProcess () function is called to run next the processes for multi programming or parallel running.
  case PRINT_STRING_OUTPUT:
  {
    // If Stdout is the screen
    if(strcmp(ProcessTable[currentProcess].output, "screen") == 0)
    {   
        // If process is not ls, It will print string directly screen.
        if(ProcessTable[currentProcess].ProcessID != 5)
          write_output (console_out, " %s", mem_reference (R[REG_A0]));
        // If process is ls, it will take string from their name list.
        // It will print this string to screen.
        // This name list keep scanned directory files.
        else
        {
          ProcessTable[currentProcess].n3 += 1;
          if(ProcessTable[currentProcess].n3 % 4 == 0)
            write_output (console_out, "%s\n", ProcessTable[currentProcess].namelist[ProcessTable[currentProcess].n2]->d_name);
          else
            write_output (console_out, "%s\t", ProcessTable[currentProcess].namelist[ProcessTable[currentProcess].n2]->d_name);
        }
    }
    else
    {
      char printbuf[1024];
      int bytesread = 0, byteswritten = 0;
      char* bp;
      // If process is not ls, It will print string directly to file.
      if(ProcessTable[currentProcess].ProcessID != 5)
        sprintf(printbuf, " %s ", (char*) mem_reference (R[REG_A0]));
      else
      {
        // If process is ls, it will take string from their name list.
        // It will print this string to file.
        // This name list keep scanned directory files.
        ProcessTable[currentProcess].n3 += 1;
        if(ProcessTable[currentProcess].n3 % 4 == 0)
          sprintf(printbuf, "%s\n", ProcessTable[currentProcess].namelist[ProcessTable[currentProcess].n2]->d_name);
        else
          sprintf(printbuf, "%s\t", ProcessTable[currentProcess].namelist[ProcessTable[currentProcess].n2]->d_name);
      }

      bytesread = strlen(printbuf);
      bp = printbuf;
      while(bytesread > 0)
      {
        while(((byteswritten = write(ProcessTable[currentProcess].fout, bp, bytesread)) == -1) && (errno = EINTR));
        if(byteswritten < 0)
          break;
        bytesread -= byteswritten;
        bp += byteswritten;
      }
    }

    swapProcess();
    break;
  }

  // This syscall is used by the ls.asm .
  // This syscall is special for ls process to scan directory.
  // Then, swapProcess () function is called to run next the processes for multi programming or parallel running.
  case SCANDIR:
  {
    ProcessTable[currentProcess].n2 += 1;
    while(ProcessTable[currentProcess].n2 < ProcessTable[currentProcess].n && ProcessTable[currentProcess].n > 0)
    {
      // It ignores files created for pipe processes because those files will be deleted.
      // It also ignore '.' and '..' directory paths.
      if(ProcessTable[currentProcess].namelist[ProcessTable[currentProcess].n2]->d_name[0] != '.' && strncmp(ProcessTable[currentProcess].namelist[ProcessTable[currentProcess].n2]->d_name, "pipecount", 9) != 0)
        break;
      else
        ProcessTable[currentProcess].n2 += 1;
    }
    // If there are no more files, it returns -1.
    if(ProcessTable[currentProcess].n2 >= ProcessTable[currentProcess].n || ProcessTable[currentProcess].n <= 0)
      R[3] = -1;

    swapProcess();
    break;
  }


    default:
      run_error ("Unknown system call: %d\n", R[REG_V0]);
      break;
    }

#ifdef _WIN32
    windowsParameterHandlingControl(1);
#endif
  return (1);
}


void
handle_exception ()
{
  if (!quiet && CP0_ExCode != ExcCode_Int)
    error ("Exception occurred at PC=0x%08x\n", CP0_EPC);

  exception_occurred = 0;
  PC = EXCEPTION_ADDR;

  switch (CP0_ExCode)
    {
    case ExcCode_Int:
      break;

    case ExcCode_AdEL:
      if (!quiet)
	error ("  Unaligned address in inst/data fetch: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_AdES:
      if (!quiet)
	error ("  Unaligned address in store: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_IBE:
      if (!quiet)
	error ("  Bad address in text read: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_DBE:
      if (!quiet)
	error ("  Bad address in data/stack read: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_Sys:
      if (!quiet)
	error ("  Error in syscall\n");
      break;

    case ExcCode_Bp:
      exception_occurred = 0;
      return;

    case ExcCode_RI:
      if (!quiet)
	error ("  Reserved instruction execution\n");
      break;

    case ExcCode_CpU:
      if (!quiet)
	error ("  Coprocessor unuable\n");
      break;

    case ExcCode_Ov:
      if (!quiet)
	error ("  Arithmetic overflow\n");
      break;

    case ExcCode_Tr:
      if (!quiet)
	error ("  Trap\n");
      break;

    case ExcCode_FPE:
      if (!quiet)
	error ("  Floating point\n");
      break;

    default:
      if (!quiet)
	error ("Unknown exception: %d\n", CP0_ExCode);
      break;
    }
}

void findID(Process* p1, char* buf)
{
  if(strcmp(buf, "sort") == 0)
    p1->ProcessID = 1;
  else if(strcmp(buf, "random") == 0)
    p1->ProcessID = 2;
  else if(strcmp(buf, "search") == 0)
    p1->ProcessID = 3;
  else if(strcmp(buf, "type") == 0)
    p1->ProcessID = -1;
  else if(strcmp(buf, "ls") == 0)
    p1->ProcessID = 5;
  else if(strcmp(buf, "Shell") == 0)
    p1->ProcessID = 6;
  else
    p1->ProcessID = -1;
}

void swapProcess()
{
  // Restore shell process
  if(checkflag == 0)
  {
    shellProc.ProgramCounter = PC;
    shellProc.state = READY;
    memcpy(&shellProc.registers, &R, sizeof(R));
  }

  // If breakc == 0, so there is no error when parsing command.
  // If there is process in process table, it will swap processes.
  if(breakc == 0 && ProcessTable.size() > 0)
  {
    int newProcess = (currentProcess + 1) % totalProcess;
    // If it is first process, it swap shell and this process.
    if(checkflag == 0)
    {
      checkflag = 1;
      currentProcess = 0;
      Restore(currentProcess);
      // I run the processes once for the first time.
      if(ProcessTable[currentProcess].run == 0)
      {
        ProcessTable[currentProcess].run = 1;
        bool continuable;
        run_program (PC, DEFAULT_RUN_STEPS, false, false, &continuable);
      }
    }
    // If any process is finished, finflag is 1.
    else if(finflag == 1 || newProcess != currentProcess)
    {
      // If the process is not finished, it updates the process information.
      if(finflag != 1 && currentProcess >= 0)
      {
        updateTable(currentProcess);
        if(background == 1)
          currentProcess = -1;
      }
      currentProcess = (currentProcess + 1) % totalProcess;
      Restore(currentProcess);
      finflag = 0;
      // I run the processes once for the first time.
      if(ProcessTable[currentProcess].run == 0)
      {
        ProcessTable[currentProcess].run = 1;
        bool continuable;
        run_program (PC, DEFAULT_RUN_STEPS, false, false, &continuable);
      }
    }
  }
  // The processes in the commands received were executed
  // or there was an error while parse,
  // so the shell process is returned to.
  else
  {
    // Brings global variables to their initial state.
    checkflag = 0;
    currentProcess = 0;
    totalProcess = 0;
    pipecount = 0;
    generate = 0;
    finflag = 0;
    cantype = 0;
    background = 0;
    breakc = 0;
    PC = shellProc.ProgramCounter ;
    shellProc.state = RUNNING;
    memcpy(&R, &shellProc.registers, sizeof(R));
  }
}

// Update process information
void updateTable(int index)
{
  ProcessTable[index].ProgramCounter = PC;
  ProcessTable[index].state = READY;
  memcpy(&ProcessTable[index].registers, &R, sizeof(R));
}

// Restore spim with new process
void Restore(int index)
{
  PC = ProcessTable[index].ProgramCounter;
  ProcessTable[index].state = RUNNING;
  memcpy(&R, &ProcessTable[index].registers, sizeof(R));
}

// Free memory of spim
void freeMemory()
{
  for (mem_addr add = kernelFinish; add < processFinish; add += 4)
  {
    free(text_seg[(add - TEXT_BOT) >> 2]);
  }
  processFinish = kernelFinish;
}

// This is not necessary. It is for test. Prints some information of process table.
void printx()
{
  int i;
  for(i = 0; i < totalProcess; i++)
  {
    printf("\n------------------\n");
    printf("%s\n%s\n%s\n%d\n", ProcessTable[i].processName, ProcessTable[i].input, ProcessTable[i].output, ProcessTable[i].ProcessID);
  }
}