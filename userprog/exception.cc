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

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>

using namespace std;

#include "exception.h"

Lock* lockTableLock = new Lock("lockTableLock");
Lock* cvTableLock = new Lock("cvTableLock");

Table lockTable(MAX_LOCKS);
Table cvTable(MAX_LOCKS);

Table processTable(MAX_NUM_PROCESSES);
Lock* processTableLock = new Lock("processTableLock");
int numOfProcess = 0;

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}

int CreateLock_Syscall(int name, int len)
{

	lockTableLock->Acquire();
	
	// create a user lock
	LockEntry* newLock = new LockEntry();
	newLock->lock = new Lock("LockEntry");
	newLock->lockSpace = currentThread->space;
	newLock->isToDelete = false;

	// put it into lock table	
	int id = lockTable.Put(newLock);
	if (id == -1) { // failed
		printf("Create lock failed.\n");
		delete newLock->lock;
		delete newLock;
	}
	printf("Create lock success. lock id is %d\n", id);
	
	lockTableLock->Release();
	return id;
}	

void DestroyLock_Syscall(int id)
{
	lockTableLock->Acquire();
	
	LockEntry* l = (LockEntry*)lockTable.Get(id);
	if(l == NULL) {
		printf("The lock %d is not exist.\n", id);
	} else {
		if (l->usrCount == 0) {
			if (l->lock != NULL) {
				delete l->lock;
			}
			delete l;
			lockTable.Remove(id);
			printf("The lock %d is being deleted.\n", id);
		} else { // someone else is still using the lock
			printf("Couldn't destroy the lock.\n");
			l->isToDelete = true;
		}
	}
	
	lockTableLock->Release();
}

void Acquire_Syscall(int id)
{
	lockTableLock->Acquire();
	
	LockEntry* l = (LockEntry*)lockTable.Get(id);
	if(l == NULL) {
		printf("The lock %d is not exist.\n", id);
		lockTableLock->Release();
	} else {
//		l->usrCount++;
		lockTableLock->Release(); // must be released before acquire
		l->lock->Acquire();
		printf("The lock %d is acquired by %s.\n", id, currentThread->getName());
	}
}

void Release_Syscall(int id)
{
	lockTableLock->Acquire();
	
	LockEntry* l = (LockEntry*)lockTable.Get(id);
	if(l == NULL) {
		printf("The lock %d is not exist.\n", id);
	} else {
//		printf("If the lock %d holded by same thread.\n", id);
		if (l->lock->owner == currentThread) {
//			l->usrCount--;
			printf("Trying to release the lock %d.\n", id);
			l->lock->Release();
		} else {
			printf("currentThread \"%s\" didn't hold the lock.\n",
			  currentThread->getName());
		}
	}
	
	lockTableLock->Release();
}	

int CreateCondition_Syscall(int name, int len)
{
	cvTableLock->Acquire();
	
	// new cv
	ConditionEntry* cv = new ConditionEntry();
	cv->cv = new Condition("Condition");
	cv->cvSpace = currentThread->space;
	cv->isToDelete = false;
	cv->usrCount = 0;

	int id = cvTable.Put(cv);
	if (id == -1) {
		printf("Create condition variable failed.\n");
		delete cv->cv;
		delete cv;
	} else {
		printf("Create condition variable successful, id is %d.\n", id);
	}
	
	cvTableLock->Release();

	return id;
}

void DestroyCondition_Syscall(int id)
{
	cvTableLock->Acquire();
	
	ConditionEntry* cv = (ConditionEntry *)cvTable.Get(id);
	if (cv == NULL) {
		printf("Condition variable %d isn't exist.\n", id);
	} else {
		if (cv->usrCount == 0) {
			delete cv->cv;
			delete cv;
			cvTable.Remove(id);
			printf("Destroying the condition variable %d.\n", id);
		} else {
			printf("Someone else is still using condition variable %d.\n", id);
			cv->isToDelete = true;
		}
	}

	cvTableLock->Release();
}

void Wait_Syscall(int lockId, int cvId)
{
	cvTableLock->Acquire();
	
	ConditionEntry* cv = (ConditionEntry *)cvTable.Get(cvId);
	LockEntry* l = (LockEntry *)lockTable.Get(lockId);
	if (cv == NULL || l == NULL) {
		printf("Lock %d isn't exist.\n", lockId);
		printf("Condition variable %d isn't exist.\n", cvId);
		cvTableLock->Release();
	} else {
//		cv->usrCount++;
		cvTableLock->Release(); // must be released before wait
		printf("Condition variable %d is waiting.\n", cvId);
		cv->cv->Wait(l->lock);
		
		// reduce the count when wake up
		cvTableLock->Acquire();
		printf("Condition variable %d is waked up by someone.\n", cvId);
//		cv->usrCount--;
		cvTableLock->Release();
	}
}

void Signal_Syscall(int lockId, int cvId)
{
	cvTableLock->Acquire();
	
	ConditionEntry* cv = (ConditionEntry *)cvTable.Get(cvId);
	LockEntry* l = (LockEntry *)lockTable.Get(lockId);
	if (cv == NULL || l == NULL) {
		printf("Lock %d isn't exist.\n", lockId);
		printf("Condition variable %d isn't exist.\n", cvId);
	} else {
		cv->cv->Signal(l->lock);
		printf("Sending signal to Condition variable %d\n", cvId);
	}

	cvTableLock->Release();
}

void Broadcast_Syscall(int lockId, int cvId)
{
	cvTableLock->Acquire();
	
	ConditionEntry* cv = (ConditionEntry *)cvTable.Get(cvId);
	LockEntry* l = (LockEntry *)lockTable.Get(lockId);
	if (cv == NULL || l == NULL) {
		printf("Lock %d isn't exist.\n", lockId);
		printf("Condition variable %d isn't exist.\n", cvId);
	} else {
		cv->cv->Broadcast(l->lock);
	}

	cvTableLock->Release();
}

void Yield_Syscall()
{
	//this system call just Yields the CPU and simulates a time slice
	printf("Yield_Syscall\n");
	currentThread->Yield();
}

void kernel_thread(unsigned int vaddr)
{

//	currentThread->space->InitRegisters();//initialize registers to 0
	
	// write virtual address to PCReg
	machine->WriteRegister(PCReg, vaddr);
	// write next instruction virtual address
	machine->WriteRegister(NextPCReg, vaddr+4);
	
	
	printf("ayush123\n");
	int stackPos = currentThread->space->StackAllocation(currentThread->threadID);//get some stack space
	
	// Write stack position
	machine->WriteRegister(StackReg, stackPos);

//	printf("kernel_thread: numPages %d\n", currentThread->space->numPages);

	// prevent information loss
	currentThread->space->RestoreState();	

//	printf("kernel_thread: numPages %d\n", currentThread->space->numPages);

	machine->Run();
}

void Fork_Syscall(unsigned int vaddr)
{
	// validation
	if ((vaddr <= 0) || ((vaddr%4) != 0)) {
		printf("Fork_Syscall: Invalid vaddr: %d\n", vaddr);
		return;
	}
	
	Thread* t = new Thread("Thread");
	
	// Get process Table
	processTableLock->Acquire();
	ProcessEntry* process = (ProcessEntry *)processTable.Get(currentThread->processID);
	if (process == NULL) {
		printf("Fork_Syscall: Impossible error. %d\n", currentThread->processID);
		return;
	}
	
	// Update the process thread info
	t->processID = currentThread->processID;
	t->threadID = process->totalNumOfThreads;
	process->totalNumOfThreads++;
	process->numOfExecutingThread++;
	
	// Update page table
	
	t->space = currentThread->space;
	
	processTableLock->Release();	

	printf("Fork_Syscall: Fork a new thread %d\n", t->threadID);

	t->Fork((VoidFunctionPtr)kernel_thread, vaddr);
}

void exec_thread(unsigned int vaddr)
{
	currentThread->space->InitRegisters();

	currentThread->space->RestoreState();
	
	machine->Run();
}

void Exec_Syscall(unsigned int vaddr, int len)
{

	if ((vaddr <= 0) || ((vaddr%4) != 0)) {
		printf("Exec_Syscall: Invalid vaddr: %d\n", vaddr);
		return;
	}
	
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("Exec_Syscall: Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

	OpenFile* exe = fileSystem->Open(buf);
	if (exe == NULL) {
		printf("Exec_Syscall: Open file %s failed.\n", buf);
		delete buf;
		return;
	}
	delete buf;
	
	// update process table
	processTableLock->Acquire();
	
	// create a process
	ProcessEntry* newProcess = new ProcessEntry();
	newProcess->totalNumOfThreads = 1;
	newProcess->numOfExecutingThread = 1;

	// put it into process table	
	int id = processTable.Put(newProcess);
	if (id == -1) { // failed
		printf("Exec_Syscall: Create process failed.\n");
		delete buf;
		delete exe;
		delete newProcess;
		return;
	}
	printf("Exec_Syscall: Create process success. process id is %d\n", id);
	newProcess->processID = id;
	
	numOfProcess++;

	processTableLock->Release();
	
	// new process main thread
	Thread* t = new Thread("Main thread");
	
	// create space for this process
	AddrSpace* space = new AddrSpace(exe);
	
	t->space = space;	
	t->threadID = 0;
	t->processID = id;
	
	// new PageTable
	
	// code & data
	
	
	delete buf;
	delete exe;

	t->Fork((VoidFunctionPtr)exec_thread, 0);
}	

void Exit_Syscall(int status)
{
	printf("Thread is exiting.\n");
	if(status != 0) {
		printf("Thread exit status: %d\n", status);
		return;
	}
	
	//
	processTableLock->Acquire();
	ProcessEntry* process = (ProcessEntry *)processTable.Get(currentThread->processID);
	if (process == NULL) {
		printf("Exit_Syscall: Impossible error. %d\n", currentThread->processID);
		return;
	}
	
	process->numOfExecutingThread--;
	numOfProcess--;
	if(process->numOfExecutingThread == 0) { // last thread
		
		processTable.Remove(currentThread->processID);
		currentThread->space->StackDeallocation(currentThread->threadID);
		delete currentThread->space;
		
		// release other resource
		
		if (numOfProcess == 0){ // last process
			processTableLock->Release();
			interrupt->Halt();		
		}
	} else {
		// delete stack pages
		currentThread->space->StackDeallocation(currentThread->threadID);
	}
	
	processTableLock->Release();
	currentThread->Finish();
	
	return;
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;

	    case SC_CreateLock:
		DEBUG('a', "Create lock syscall.\n");
		rv = CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_DestroyLock:
		DEBUG('a', "Destroy lock syscall.\n");
		DestroyLock_Syscall(machine->ReadRegister(4));
		break;
	    case SC_Acquire:
		DEBUG('a', "Acquire lock syscall.\n");
		Acquire_Syscall(machine->ReadRegister(4));
		break;
	    case SC_Release:
		DEBUG('a', "Release lock syscall.\n");
		Release_Syscall(machine->ReadRegister(4));
		break;

	    case SC_CreateCondition:
		DEBUG('a', "Create condition variable syscall.\n");
		rv = CreateCondition_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_DestroyCondition:
		DEBUG('a', "Destroy condition variable syscall.\n");
		DestroyCondition_Syscall(machine->ReadRegister(4));
		break;
	    case SC_Wait:
		DEBUG('a', "Wait condition variable syscall.\n");
		Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Signal:
		DEBUG('a', "Signal condition variable syscall.\n");
		Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Broadcast:
		DEBUG('a', "Broadcast condition variable syscall.\n");
		Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;

	    case SC_Yield:
		DEBUG('a', "Yield thread syscall.\n");
		Yield_Syscall();
		break;
	    case SC_Exec:
		DEBUG('a', "Exec process syscall.\n");
		Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Fork:
		DEBUG('a', "Fork thread syscall.\n");
		Fork_Syscall(machine->ReadRegister(4));
		break;
	    case SC_Exit:
		DEBUG('a', "Exit thread syscall.\n");
		Exit_Syscall(machine->ReadRegister(4));
		break;
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
