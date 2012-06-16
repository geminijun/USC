/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"


int a[3];
int b, c, d;
char buf[12]="Customer_cv";


int
main()
{
	/* Creating 3 condition variables.
	/*CreateLock("Customer", 8);*/
	/*for(b=0;b<500;b++){  //Loop to fill the condition variable Table and test the behavior. */
	CreateCondition(buf, sizeof(buf));
	/*}*/
	CreateCondition("Testfile_cv", 12);
	CreateCondition("Cashier_cv", 12);
	
	/* Deleting a condition variable at index 1 in condition variable table */
	DestroyCondition(1);
	DestroyCondition(1);
	CreateCondition("Testfile_cv", 12);
	
	/* Deleting a condition variable at index out of range in condition variable table */
	DestroyCondition(-501);
	
	/* Displaying all the existing condition variables in Lock table */
	/* DisplayCV(); */
	
	/* Creating 2 locks */
	c=CreateLock("Cust",4);
	CreateLock("Cash",4); 		/*lockId ==1 */
	
	/* Acquiring  first lock */
	Acquire(c);
	
	/* Signalling on a lock */
	Signal(b,1);
	
	/* Broadcasting on a lock */
	Broadcast(b,1);
	
	/* Trying to Wait on a lock which has not been Acquired */
	Wait(b,1);
	
	/* Trying to Wait on a condition variable which is already busy with another lock*/
	Wait(b,0);
	
	/* Waiting on an Acquired lock */
	Wait(1,c);

}
