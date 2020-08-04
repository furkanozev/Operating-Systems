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
#include "data.h"
#include "spim-utils.h"

#include <vector>
#include <string.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
using namespace std;

enum ProcessState {
  READY,
  WAITING,
  RUNNING
};

struct Process{
  char processName[16];
  int ProcessID;
  int ParentProcessID;
  mem_addr ProgramCounter;
  ProcessState state;
  reg_word registers[R_LENGTH];
};

int maxProcess = 1;
int currentProcess = 0;
int totalProcess = 0;
int forkProcess = 0;
int processFlag = 0;

mem_addr kernelFinish;
mem_addr processFinish;

vector <Process> ProcessTable;

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


/*
  If amount of fork procosses is equal to maximum process number, It will start to change process when interrupt comes.
  Otherwise, Kernel process will continue.

  If any process is terminate, it will not update
  Otherwise update process.

  Then find next ready process, then switch them using Restore() function.
  Print new running process informatin using printProcess() functionç
  Run new process.
*/
void SPIM_timerHandler()
{
   // Implement your handler..
   try
   {
    if(totalProcess == maxProcess && ProcessTable.size() != 0){
      if(processFlag == 1) processFlag = 0;
      else updateTable(currentProcess);

      currentProcess = findReadyProcess();
      Restore(currentProcess);

      printProcess();

      bool continuable;
      run_program (PC, DEFAULT_RUN_STEPS, false, false, &continuable);
    }
   }
   catch ( exception &e )
   {
      cerr <<  endl << "Caught: " << e.what( ) << endl;
   };
   
}
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
      spim_return_value = R[REG_A0];  /* value passed to spim's exit() call */
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
      /*
        When any process is terminated, it's process information is deleted from process table.
      */
      case POSIX_PROCESS_EXIT:
      {
        if(ProcessTable.size() == 0)
        {
          spim_return_value = 0;
          return (0);
        }
        else if(currentProcess == 0){
          ProcessTable.erase(ProcessTable.begin());
          spim_return_value = 0;

          freeMemory();

          cout<<"\n=> Process Table Size: "<<ProcessTable.size();
          cout<<"\n\n----- ALL PROCESS ARE TERMINATED -----\n\n";

          return (0);
        }
        else{
          ProcessTable.erase(ProcessTable.begin() + currentProcess);
          // When process is changed, It will find next process. But in this condition, one process is terminate and another process will be passed.
          // So, x 1 is reduced to change to the next process.
          currentProcess -= 1;
          // If process is deleted, so it must not update because process is gone. processFlag is used on timeHandler function for this checking.
          processFlag = 1;
          // For process changing, So I used timerHandler function instead of write new function.
          SPIM_timerHandler();
        }
        break;
      }

      /*
        When kernel is loaded it will start a process named init with process id 0.
        Save the information about this process.
        This process will be added in ProcessTable.
        For finish kernel, it will need to current program counter. It will used to terminate init process.
        totalProcess, forkProcess will be start as 1.
        When init syscall calling, it take number of process as parameter.
      */
      case INIT_SYSCALL:
      {
        Process init;

        strcpy(init.processName, "init");
        init.ProcessID = 0;
        init.ParentProcessID = 0;
        init.ProgramCounter = PC;
        init.state = RUNNING;
        memcpy(&init.registers, &R, sizeof(R));

        ProcessTable.push_back(init);

        kernelFinish = current_text_pc();

        maxProcess += R[REG_A0];
        totalProcess = 1;
        forkProcess = 1;

        break;
      }

      /*
        Firstly, update init process(kernel) information.
        When a process is to be forked, the init process is copied and added to the ProcessTable.
        We don't know process name yet.
        Process ID must be unique, so I used processTable size for new process id.
        State of init process is running so it must be ready. Two processes cannot work at the same time.
        The state of the new process is set to ready to prevent conflict.
        Increase forkProcess number to check to reach maximum process.
        It keep to finish program counter (processFinish). When all process are terminated, it will use to exit and free memory.
        If child process is created, return child process id via $v0.
        Otherwise return -1 via $v0.
      */
      case FORK_SYSCALL:
      {
        if(forkProcess < maxProcess)
        {
          updateTable(currentProcess);

          Process child;

          strcpy(child.processName, "");
          child.ProcessID = ProcessTable.size();
          child.ParentProcessID = currentProcess;
          child.state = READY;
          child.ProgramCounter = PC;
          memcpy(&child.registers, &R, sizeof(R));

          ProcessTable.push_back(child);

          forkProcess += 1;
          processFinish = current_text_pc();

          R[REG_V0] = child.ProcessID;
        }
        else
        {
          R[REG_V0] = -1;
        }
        break;
      }

      /*
        When execve syscall calling, it take number of file.
        Then selection function returns the name of the file.
        Then load assembly file using read_assembly_file function.
        Then update process information. (PC, Register, Process Name)
        Increase totalProcess.
        If Image of process is not loaded, return -1 via $v0.
      */
      case EXECVE_SYSCALL:
      {
        if(totalProcess < maxProcess)
        {
          char* fileName = Selection(R[REG_A0]);
          mem_addr processPC = current_text_pc();
          initialize_symbol_table();
          read_assembly_file(fileName);
          loadImage(readyLoad(), fileName, processPC);

          totalProcess += 1;
        }
        else
        {
          R[REG_V0] = -1;
        }
        break;
      }

      /*
        When all process are terminated, waitpid syscall return -1.
        Then infinite loop in kernel will terminate.

        If images of all processes are loaded and these programs are terminated, Then it will return -1 to break loop in kernel.
      */
      case WAITPID_SYSCALL:
      {
        if(totalProcess == maxProcess && ProcessTable.size() == 1)
        {
          R[REG_V0] = -1;
        }
        break;
      }

      /*
        This sycall generate random integer number.
        It take an parameter to get mod via $a0.
        Return random number via $v0.
      */
      case GENERATE_RANDOM_NUMBER:
      {
        int number;
        int mod = R[REG_A0];

        srand (time(NULL));

        number = rand() % mod;

        R[REG_V0] = number;

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

/*
  To update process,
  Save last program counter, change state as ready and copy register.
*/
void updateTable(int index) {
  ProcessTable[index].ProgramCounter = PC;
  ProcessTable[index].state = READY;
  memcpy(&ProcessTable[index].registers, &R, sizeof(R));
}

/*
  To restore process,
  Load program counter of new process, change state as running and load register from new process register.
*/
void Restore(int index) {
  PC = ProcessTable[index].ProgramCounter;
  ProcessTable[index].state = RUNNING;
  memcpy(&R, &ProcessTable[index].registers, sizeof(R));
}

/*
  Every time a timer interrupt occurs, there is a chance to make a process switch.
  Round Robin scheduling is used.
  Find next ready process.
  If it can not find it when it has come to an end, it returns to the beginning and it continue to find. It is like a list.
  If there are no processes other than init, the sequence passes to init process.
*/
int findReadyProcess() {
  if(ProcessTable.size() == 1) return 0;
  int i = (currentProcess + 1) % ProcessTable.size();
  while((i != currentProcess && ProcessTable[i].state != READY) || i == 0)
    i = (i + 1) % ProcessTable.size();
  return i;
}

// Print all information of current running process.
void printProcess() {
  Process current = ProcessTable[currentProcess];

  cout<<"\n\n----- PROCESS INFORMATION -----\n"
      <<" Process ID: "<<current.ProcessID
      <<"\n Process Parent ID: "<<current.ParentProcessID
      <<"\n Process Name: "<<current.processName
      <<"\n Program Counter: "<<current.ProgramCounter
      <<"\n Stack Pointer Address: "<<current.registers[REG_SP];

  if(current.state == READY) cout<<"\n Process State: READY\n";
  else if(current.state == WAITING) cout<<"\n Process State: WAITING\n";
  else cout<<"\n Process State: RUNNING\n";

  cout<<"\n=> Process Table Size: "<< ProcessTable.size()<<" Current Process ID: "<<currentProcess<<"\n";
  cout<<"-------------------------------\n";

}

// Return File name.
char* Selection(int program)
{
  switch (program)
  {
  case 1:
    return "LinearSearch.s";
  case 2:
    return "BinarySearch.s";
  default:
    return "Collatz.s";
  }
}

/*
  Finds the first process whose image is not loaded.
  If the process has no name, the image is not loaded.
*/
int readyLoad() 
{
  unsigned int i;
  for (i = 1; i < ProcessTable.size(); i++)
  {
    if (strcmp(ProcessTable[i].processName, "") == 0)
      break;
  }
  return i;
}


//To load image, change process name, program counter and update registers.
void loadImage(int index, char* name, mem_addr pc_)
{
  strcpy(ProcessTable[index].processName, name);
  ProcessTable[index].ProgramCounter = pc_;
  memcpy(&ProcessTable[index].registers, &R, sizeof(R));
}

/* 
  When init process(kernel) is terminated, it will free memory of other process.
  Thus, emulator will shut down only after all the programs in memory terminate.
*/
void freeMemory()
{
  for (mem_addr add = kernelFinish; add < processFinish; add += 4)
  {
    free(text_seg[(add - TEXT_BOT) >> 2]);
  }
}