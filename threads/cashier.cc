#include "synch.h"
#include "system.h"
#include "cashier.h"

void cashier_start(int id)
{
	while(1) {
		// check if sended to break by manager

		// get the lock of the line

		// wake up one customer form the list

		// calculate the bill

		// wait for pay

		if(isEnough()) {
			// tell customer leave
		} else {
			// call manager
		}
	}
}
