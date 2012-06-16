#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "synch.h"

extern Lock* lockTableLock;
extern Lock* cvTableLock;

extern Table processTable;

struct LockEntry {
	Lock* lock;
	AddrSpace* lockSpace;
	bool isToDelete;
	int usrCount;
};

struct ConditionEntry {
	Condition* cv;
	AddrSpace* cvSpace;
	bool isToDelete;
	int usrCount;
};

const int MAX_LOCKS=500;
// LockEntry lockTable[MAX_LOCKS];
extern Table lockTable;
extern Table cvTable;


const int MAX_NUM_PROCESSES = 1024;

struct ProcessEntry{
	int processID;
	int totalNumOfThreads;	// 
	int numOfExecutingThread;	//
};


#endif
