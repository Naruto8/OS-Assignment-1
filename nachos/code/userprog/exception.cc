// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#include "scheduler.h"
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "console.h"
#include "synch.h"
#include "machine.h"
#include "translate.h"
#include "utility.h"
#include "addrspace.h"
#include "stats.h"
#include "thread.h"
#include "switch.h"
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
static Semaphore *readAvail;
static Semaphore *writeDone;
static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

void InitialFunction(int arg){

      // If the old thread gave up the processor because it was finishing,
      // we need to delete its carcass.  Note we cannot delete the thread
      // before now (for example, in NachOSThread::FinishThread()), because up to this
      // point, we were still running on the old thread's stack!
      if (threadToBeDestroyed != NULL) {
        delete threadToBeDestroyed;
        threadToBeDestroyed = NULL;
       }
          
      #ifdef USER_PROGRAM
          if (currentThread->space != NULL) {   // if there is an address space
              currentThread->RestoreUserState();     // to restore, do it.
        currentThread->space->RestoreStateOnSwitch();
          }
      #endif
      machine->Run();
}

static void ConvertIntToHex (unsigned v, Console *console)
{
   unsigned x;
   if (v == 0) return;
   ConvertIntToHex (v/16, console);
   x = v % 16;
   if (x < 10) {
      writeDone->P() ;
      console->PutChar('0'+x);
   }
   else {
      writeDone->P() ;
      console->PutChar('a'+x-10);
   }
}

void
ExceptionHandler(ExceptionType which)
{
	// printf("%d\n", type);
	if(currentThread ==  NULL)
		printf("NO currentThread\n");
	int type = machine->ReadRegister(2);
	int memval, vaddr, printval, tempval, exp;
	unsigned printvalus;        // Used for printing in hex
	if (!initializedConsoleSemaphores) {
		readAvail = new Semaphore("read avail", 0);
		writeDone = new Semaphore("write done", 1);
		initializedConsoleSemaphores = true;
	}
	Console *console = new Console(NULL, NULL, ReadAvail, WriteDone, 0);;

	if ((which == SyscallException) && (type == SYScall_Halt)) {
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
	}
	else if ((which == SyscallException) && (type == SYScall_PrintInt)) {
		printval = machine->ReadRegister(4);
		if (printval == 0) {
			writeDone->P() ;
			console->PutChar('0');
		}
		else {
			if (printval < 0) {
				writeDone->P() ;
				console->PutChar('-');
				printval = -printval;
			}
			tempval = printval;
			exp=1;
			while (tempval != 0) {
			tempval = tempval/10;
			exp = exp*10;
		}
		exp = exp/10;
		while (exp > 0) {
			writeDone->P() ;
			console->PutChar('0'+(printval/exp));
			printval = printval % exp;
			exp = exp/10;
		}
	}
	// Advance program counters.
	machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}
	else if ((which == SyscallException) && (type == SYScall_PrintChar)) {
		writeDone->P() ;
		console->PutChar(machine->ReadRegister(4));   // echo it!
		// Advance program counters.
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}
	else if ((which == SyscallException) && (type == SYScall_GetReg)) {
		machine->WriteRegister(2,machine->ReadRegister(machine->ReadRegister(4)));	
		// Advance program counter
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}
	else if ((which == SyscallException) && (type == SYScall_Fork)){
		NachOSThread *child = new NachOSThread("forkChild");
		child->SetPPID(currentThread->GetPID());

		int parentsize = currentThread->space->VMpageSize()*PageSize;
		ProcessAddrSpace *childspace = new ProcessAddrSpace(parentsize);

		currentThread->childList[currentThread->numberOfChild] = child->GetPID();
		currentThread->numberOfChild++;
		//Allocate space to child
		child->space = childspace;

		//Set child's starting address
		child->space->StartingAddress = currentThread->space->StartingAddress + parentsize +1;
		//Copy contents to child
		currentThread->CopyAddressSpaceToChild(child);
		// Advance program counter
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);

		//saving process register of child
		machine->WriteRegister(2,0);
		child->SaveUserState();

		//returning PID of child to parent
		machine->WriteRegister(2,child->GetPID());

		//initial function
		child->ThreadFork(InitialFunction, 0);
	}
	else if ((which == SyscallException) && (type == SYScall_Join)){
		int childPID = machine->ReadRegister(4);
		bool flag= false;
		for(int i=0; i< currentThread->numberOfChild; i++){
			if(childPID == currentThread->childList[i])
				flag = true;
		}
		if(flag == false) 	machine->WriteRegister(2,-1);
		else{
			if(scheduler->exitCode[childPID] != -1000){
				machine->WriteRegister(2,scheduler->exitCode[childPID]);
			}
			else{
				scheduler->WaitingForChild->Append(currentThread);
				IntStatus oldLevel = interrupt->SetLevel(IntOff);
				currentThread->PutThreadToSleep();
				(void)interrupt->SetLevel(oldLevel);
			}
		}
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}
	else if((which == SyscallException) && (type == SYScall_Exit)){
		if(currentThread->GetPID() == 0){
			interrupt->Halt();
		}
		if(currentThread != NULL){
			int parentID = currentThread->GetPPID();
			NachOSThread *curr;
			int first , key ;
			if(!scheduler->WaitingForChild->IsEmpty()){
				curr =(NachOSThread *)scheduler->WaitingForChild->Remove();
				first = curr->GetPID();
				key = first;
				do{
					if(key == parentID){
						scheduler->ThreadIsReadyToRun(curr);
						break;
					}
					else {
						scheduler->WaitingForChild->Append(curr);
						curr=(NachOSThread * )scheduler->WaitingForChild->Remove();
						key = curr->GetPID();
						if(key == first)
							break;
					}
				}while(1);
			}
		}
		if(currentThread != NULL)
			scheduler->exitCode[currentThread->GetPID()] = machine->ReadRegister(4);
		currentThread->FinishThread();

		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);		
	}
	else if ((which == SyscallException) && (type == SYScall_NumInstr)) {
		machine->WriteRegister(2, machine->NumInstr());
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}
	else if ((which == SyscallException) && (type == SYScall_GetPA)) {
		int virtAddr = machine->ReadRegister(4);
		unsigned int vpn,offset,pageFrame;
		TranslationEntry *entry;
		vpn = (unsigned)virtAddr/PageSize;
		offset = (unsigned) virtAddr%PageSize;

		if(vpn>machine->pageTableSize) {
			machine->WriteRegister(2,-1);
			machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
			machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
			machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
		}
		else if (!machine->NachOSpageTable[vpn].valid){
			machine->WriteRegister(2,-1);
			machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
			machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
			machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
		}
		entry = &(machine->NachOSpageTable[vpn]);
		pageFrame = entry->physicalPage;
		if(pageFrame>=NumPhysPages){
			machine->WriteRegister(2,-1);
			machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
			machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
			machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
		}    
		machine->WriteRegister(2,(pageFrame*PageSize + offset));
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}
	else if((which == SyscallException) && (type == SYScall_GetPID)){
		//ASSERT(0);
		if(currentThread != NULL)
			machine->WriteRegister(2, currentThread->GetPID());
		// Advance program counters.
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}
	else if((which == SyscallException) && (type == SYScall_GetPPID)){
		machine->WriteRegister(2, currentThread->GetPPID());
		// Advance program counters.
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}
	else if ((which == SyscallException) && (type == SYScall_Time)) {
		machine->WriteRegister(2,stats->totalTicks);
		// Advance program counter
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}
	else if ((which == SyscallException) && (type == SYScall_Sleep)) {
		int WakeUpTime = machine->ReadRegister(4)+(stats->totalTicks);
		if(WakeUpTime == 0) currentThread->YieldCPU();       
		else{
			scheduler->ThreadSleep(currentThread,WakeUpTime);
			IntStatus oldLevel = interrupt->SetLevel(IntOff);
			currentThread->PutThreadToSleep();
				(void)interrupt->SetLevel(oldLevel);
		}
		// Advance program counter
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}
	else if ((which == SyscallException) && (type == SYScall_Yield)) {
		NachOSThread *nextThread;
		nextThread = scheduler->FindNextThreadToRun();
		if(nextThread != NULL){
			scheduler->Schedule(nextThread);
		}
		// Advance program counter
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}
	else if ((which == SyscallException) && (type == SYScall_PrintString)) {
		vaddr = machine->ReadRegister(4);
		machine->ReadMem(vaddr, 1, &memval);
		while ((*(char*)&memval) != '\0') {
			writeDone->P() ;
			console->PutChar(*(char*)&memval);
			vaddr++;
			machine->ReadMem(vaddr, 1, &memval);
		}
		// Advance program counters.
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}

	else if ((which == SyscallException) && (type == SYScall_PrintIntHex)) {
		printvalus = (unsigned)machine->ReadRegister(4);
		writeDone->P() ;
		console->PutChar('0');
		writeDone->P() ;
		console->PutChar('x');
		if (printvalus == 0) {
			writeDone->P() ;
			console->PutChar('0');
		}
		else {
			ConvertIntToHex (printvalus, console);
		}
		// Advance program counters.
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
	}
	else {
		printf("Unexpected user mode exception %d %d\n", which, type);
		ASSERT(FALSE);
	}
}
