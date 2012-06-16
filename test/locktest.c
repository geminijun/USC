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
int b, c;
char buf[14]="Customer_lock";


int
main()
{
	/* Creating 3 locks. 2 locks with same name but different way 
	/*CreateLock("Customer", 8);*/
	/*for(b=0;b<100;b++){  //Loop to fill the Lock Table and test the behavior. */
	CreateLock(buf, sizeof(buf));
	/*}*/
	b = CreateLock("Testfile_lock", 13);
	c = CreateLock("Cashier_lock", 13);
	
	/* Deleting a lock at index 1 in Lock table */
	DestroyLock(1);
	
	/* Deleting a lock at index out of range in Lock table */
	DestroyLock(-501);
	
	/* Displaying all the existing locks in Lock table */
	/* DisplayLock(); */
	
	/* Trying to Acquire the same lock twice (index 0) in Lock table */
	Acquire(b);
	Acquire(c);
	
	/* Trying to Acquire deleted lock at index 1 */
	Acquire(1);
	
	/* Acquiring lock at index 2 */
	Acquire(2);

	/* Acquiring a lock at index out of range in Lock table */
	Acquire(501);
	
	/* Trying to Release the same lock twice (index 0) in Lock table */
	Release(0);
	Release(0);
	
	/* Trying to Release deleted lock at index 1 */
	Release(1);
	
	/* Releasing lock at index 2 */
	Release(2);

	/* Releasing a lock at index out of range in Lock table */
	Release(501);

}
