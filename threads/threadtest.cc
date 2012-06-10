//	testing the Super Market Simulation 
//

#include "copyright.h"
#include "system.h"
#include <stdlib.h>

extern void ThreadTest(void);
extern void DisplayData(void);

// defining all the maximum allowable values for different entities.
#define MAX_NO_SALESMAN 3
#define MAX_NO_CUSTOMER 30
#define MAX_NO_GOODSLOADER 5
#define MAX_NO_DEPARTMENT 5
#define MAX_NO_TROLLY 8
#define NO_OF_ITEMS 10
#define MAX_NO_CASHIER 5



#ifdef CHANGED
#include "synch.h"
#endif

#ifdef CHANGED


// --------------------------------------------------
// Super Market Simulation
// --------------------------------------------------


// --------------------------------------------------
// declaration of global variables
// --------------------------------------------------
Lock *salesmanLock[MAX_NO_DEPARTMENT][MAX_NO_SALESMAN];
Condition *salesmanCV[MAX_NO_DEPARTMENT][MAX_NO_SALESMAN];
int salesmanStatus[MAX_NO_DEPARTMENT][MAX_NO_SALESMAN];    // a monitor variable to check the status of the salesman 1 is busy, 0 is free, 2 is OFB, 3 is ready to help  
int custNumber[MAX_NO_DEPARTMENT][MAX_NO_SALESMAN];
int custStatus[MAX_NO_DEPARTMENT][MAX_NO_SALESMAN];            // variable for salesman to check is the customer is priviledge or normal
int itemLeft[MAX_NO_DEPARTMENT][NO_OF_ITEMS];
int itemPrice[MAX_NO_DEPARTMENT][NO_OF_ITEMS];


Lock *deptShoppingLock[MAX_NO_DEPARTMENT];
Condition *deptShoppingCV[MAX_NO_DEPARTMENT];

// Msg lock for cashier which can be used by Manager or Customer
Lock* cashierMsgLock[MAX_NO_CASHIER];
// record current customer who is interacting with cashier
int currentCashierCus[MAX_NO_CASHIER];
// record current customer's purchased items
int currentCashierItems[MAX_NO_CASHIER][NO_OF_ITEMS];
// record the number of current customer's purchased items
int currentCashierItemsNo[MAX_NO_CASHIER];
// recored the total price need to be payed by customer
int currentCashierMoney[MAX_NO_CASHIER];


Lock* custCashierWaitingLineLock;
//Lock *custCashierWaitingLineLock[MAX_NO_CASHIER]; //lock variable 1 for each cashier line.
Condition *custCashierWaitingLineCV[MAX_NO_CASHIER]; // condition variable 1 for each customer waiting line
Condition *cashierCV[MAX_NO_CASHIER]; // condition variable for each cashier waiting
int custCashierWaitingLineCount[MAX_NO_CASHIER]; // waiting line for each cashier 
int cashierStatus[MAX_NO_CASHIER]; // monitor variable for tracking the status of cashiers . 0 means free, 1 means busy and 2 means out of break. 

Lock *priviledgeCustWaitingLineLock;
Condition *priviledgeCustWaitingLineCV;
int priviledgeCustWaitingLineLength = 0;
List* priviledgeCustWaitingLine;

//int privCashierWaitingLineCount = 0;
Lock findMinCashierLine("fcwmlc");
int temparray[5] = {0,0,0,0,0};

Lock findTrollyLock("findTrollyLock");
Condition findTrollyCV("findTrollyCV");
int trollyLeft = MAX_NO_TROLLY;


// the sales money for each counter
int counter[MAX_NO_CASHIER];


// total sales money
Lock* salesMoneyLock;
int salesMoney;

// lock and condition variable for manager
Lock* managerLock;
Condition* managerCV;
int managerstatus;  // 0 means free, 1 means interacting with 


// --------------------------------------------------
// Customer - Salesman - Monitor (Functions)
// --------------------------------------------------



// --------------------------------------------------
// Monitor :: declaring locks/condition/monitor variables
// --------------------------------------------------
						  
Lock *custWaitingLineLock[MAX_NO_DEPARTMENT];		  // the lock shared between customer and salesman
Condition *custWaitingLineCV[MAX_NO_DEPARTMENT];     // a condition variable for a waiting customer
int custWaitingLineCount[MAX_NO_DEPARTMENT];                 // a monitor variable to check the number of customers queued up 

//int salesmanStatus[MAX_NO_SALESMAN] = {1,1,1                  
//int salesmanID = 0; // for tracking the ID of salesman
//int i =0; // counter for FOR loop

// --------------------------------------------------
// thread_customer() -- thread for new customer entering the store
// --------------------------------------------------

void thread_customer(int myIndex) {
    int i;
	int flag = 0;
	int mySalesmanID;
	int myCashier=0;
	int tempMin=0;
	int myStatus = 0;    // this variable store whether customer is normal or priviledged
	int myDepartment = 0; // this variable store the department number of the customer 
	int myItemCount = 0; // this variable stores how many items a customer wants to buy
	int myItemsIDs[NO_OF_ITEMS];     
	
	// every new customer entering the store tries to get the lock for interacting with the salesman.
	myStatus = (rand() % 3);  
	myDepartment = (rand() % 5)+1;
	myItemCount = (rand() % 10) + 1;
	
	//Customer first gets the trolly
	findTrollyLock.Acquire();
	while (true) {
		if(trollyLeft > 0)
		{
			printf("Customer [%d] has a trolly\n", myIndex+1); 
			trollyLeft--;
			break;
		}
		else
		{
			printf("Customer [%d] gets in line for a trolly", myIndex+1);
			findTrollyCV.Wait(&findTrollyLock);
		}
	}
	findTrollyLock.Release();
	
	
	
	//temparray[myDepartment-1]++;
	custWaitingLineLock[myDepartment-1]->Acquire();
	
	if(myStatus == 0 || myStatus == 2)
	{
		printf("Customer [%d] enters the SuperMarket\n", myIndex+1); 
		//printf("Customer [%d] wants to by [%d] no.of items\n", myIndex+1,myItemCount); 
		//printf("Customer [%d] gets in line for a trolly\n", myIndex+1); 
		//printf("Customer [%d] has a trolly\n", myIndex+1); 
		printf("Customer [%d] wants to shop in Department [%d] \n", myIndex+1,myDepartment); 
	}
	else
	{
		printf("PriviledgedCustomer [%d] enters the SuperMarket\n", myIndex+1); 
		//printf("PriviledgedCustomer [%d] wants to by [%d] no.of items\n", myIndex+1,myItemCount); 
		//printf("PriviledgedCustomer [%d] gets in line for a trolly\n", myIndex+1); 
		//printf("PriviledgedCustomer [%d] has a trolly\n", myIndex+1); 	
		printf("PriviledgedCustomer [%d] wants to shop in Department [%d] \n", myIndex+1,myDepartment); 
	}
	
	for(i = 0 ; i<MAX_NO_SALESMAN ; i++)
	{
		if(salesmanStatus[myDepartment-1][i] == 1 || salesmanStatus[myDepartment-1][i] == 3)
		{
			flag++;
		}
	}

	if(flag == MAX_NO_SALESMAN)
	{
		if(myStatus == 0 || myStatus == 2)
		{
		printf("Customer [%d] gets in line gets in line for the Department [%d]\n", myIndex+1, myDepartment); 
		}
		else
		{
		printf("PriviledgedCustomer [%d] gets in line gets in line for the Department [%d]\n", myIndex+1, myDepartment); 
		}
		//printf("customer %d: ALl salesman are busy , I will wait till my turn comes up\n", myIndex+1);
		custWaitingLineCount[myDepartment-1]++;
		custWaitingLineCV[myDepartment-1]->Wait(custWaitingLineLock[myDepartment-1]);

	}
	//custWaitingLineLock.Release();	
	
	for(i = 0 ; i<MAX_NO_SALESMAN ; i++)
	{
		if(salesmanStatus[myDepartment-1][i] == 3)
		{
			
			mySalesmanID = i;
			break;
		}
		
			/*
		if(salesmanStatus[i] == 0)
		{
			printf("no");			
			mySalesmanID = i;
			salesmanCV[mySalesmanID]->Signal(salesmanLock[mySalesmanID]);
			printf("customer %d: salesman are sleeping , I will wake them up\n", myIndex+1);
			custWaitingLineCount++;
			custWaitingLineCV.Wait(&custWaitingLineLock);
			i--;
		}*/
		
	}
	
	salesmanLock[myDepartment-1][mySalesmanID]->Acquire();
	custWaitingLineLock[myDepartment-1]->Release();
	if(myStatus == 0 || myStatus == 2)
	{
		printf("Customer [%d] is  interacting with DepartmentSalesman[%d] \n", myIndex+1,mySalesmanID+1); 
	}
	else
	{
		printf("PriviledgedCustomer [%d] is  interacting with DepartmentSalesman[%d] \n", myIndex+1,mySalesmanID+1); 
	}
	//printf("customer %d: salesman %d is free, woah my turn came\n", myIndex+1, mySalesmanID+1);
	salesmanStatus[myDepartment-1][mySalesmanID] = 1;
	
	custNumber[myDepartment-1][mySalesmanID]=myIndex;
	custStatus[myDepartment-1][mySalesmanID]=myStatus;
	salesmanCV[myDepartment-1][mySalesmanID]->Signal(salesmanLock[myDepartment-1][mySalesmanID]);
	salesmanCV[myDepartment-1][mySalesmanID]->Wait(salesmanLock[myDepartment-1][mySalesmanID]);
		
	// greeting with salesman
	
		//salesmanStatus = 0;
	//salesWaitingCV.Signal(&custsalesLock);
//	printf ("%d: I have entered the super-market\n", myIndex+1);
	salesmanCV[myDepartment-1][mySalesmanID]->Signal(salesmanLock[myDepartment-1][mySalesmanID]);
	salesmanLock[myDepartment-1][mySalesmanID]->Release();
		
	
	
	//At this point the cutomers successfully enters the supermarket and has been greeted by the department salesman.
	//Now at a time in one department only customer can do shopping. so they acquire the department specific lock
	deptShoppingLock[myDepartment-1]->Acquire();
	// the customer can now see all the items remaining in the shelves/aisle
	
	//printf("Customer [%d] .. [%d] no ..... department [%d] ..items", myIndex+1,myItemCount, myDepartment);
	for(i=0;i<myItemCount;i++)
	{
		
		int temp;
		temp = rand() % 10;
		myItemsIDs[i]=temp;
		if(itemLeft[myDepartment-1][temp]>0)
		{
			itemLeft[myDepartment-1][temp]--;
			//printf(" [%d]  ", temp+1);
		}
		else
		{
			//signal salesman -> goods loader -> replenishes blah blah blah;
			printf("Customer [%d] i couldnt pick up item no [%d] as it is finished\n ", myIndex+1, temp+1);
		}
		
		
	}
	printf("\n");
	//Releasing the lock so that other customer from the same department can now proceed with his shopping.
	deptShoppingLock[myDepartment-1]->Release();
	
	//At this point the current customer has shopped all the items that he wishes to buy. and the items are in his trolly
	//no he wishes to determine the which cashier has the lowest number of people in his line.
	//but for the priviledge customer, he enters the common line.
	
/*	if (myStatus == 1) {
		priviledgeCustWaitingLineLock->Acquire();
		priviledgeCustWaitingLine->Append(&myIndex);
		priviledgeCustWaitingLineLength++;
		priviledgeCustWaitingLineCV->Wait(priviledgeCustWaitingLineLock);

		myCashier = 
		cashierCV[myCashier]->Signal(custCashierWaitingLineLock[myCashier]);
		custCashierWaitingLineLock[myCashier]->Release();

	} else {		*/
		findMinCashierLine.Acquire();
		tempMin=custCashierWaitingLineCount[0];
		myCashier=0;
		for(i=0;i<MAX_NO_CASHIER;i++)
		{
			if(custCashierWaitingLineCount[i]<tempMin)
			{
				tempMin=custCashierWaitingLineCount[i];
				myCashier = i;
			}
		}
		
		//printf("[%d] my cashier is [%d] \n",myIndex+1, myCashier+1);
		custCashierWaitingLineLock[myCashier]->Acquire();
		printf("[%d] my cashier is [%d] \n",myIndex+1, myCashier+1);
		custCashierWaitingLineCount[myCashier]++;
		findMinCashierLine.Release();
		
		if(cashierStatus[myCashier] == 0)
		{
			cashierStatus[myCashier] = 1;
//			cashierCV[myCashier]->Signal(custCashierWaitingLineLock[myCashier]);
		}
		
		if(cashierStatus[myCashier] == 1)
		{
			printf("Customer [%d] i am waiting in the line of cashier [%d]\n",myIndex+1,myCashier+1 );
			custCashierWaitingLineCV[myCashier]->Wait(custCashierWaitingLineLock[myCashier]);
		}
		
		cashierCV[myCashier]->Signal(custCashierWaitingLineLock[myCashier]);

		custCashierWaitingLineLock[myCashier]->Release();

//	}

	// set the global variables so cashier can use to calculate the money
	currentCashierItemsNo[myIndex] = myItemCount;
	for(i=0;i<myItemCount;i++)
	{
		currentCashierItems[myIndex][i] = myItemsIDs[i];
	}
	
	// inform cashier to pay
	cashierCV[myCashier]->Signal(custCashierWaitingLineLock[myCashier]);
		
	// waiting for cashier's scanning
	cashierCV[myCashier]->Wait(custCashierWaitingLineLock[myCashier]);
	
	if (currentCashierMoney[myCashier] <= 400) {
		printf("Customer [%d] pays [%d] to Cashier [%d] and is now waiting for receipt.", myIndex, needMoney, myCashier);
		cashierCV[myCashier]->Wait(custCashierWaitingLineLock[myCashier]);	
		printf("Customer [%d] got receipt from Cashier [%d] and is now leaving", myIndex, myCashier);
	} else {
		printf("Customer [%d] cannot pay [%d]", myIndex, needMoney);
		// don't have enough money, signal manager to process
		// reduce the bill until enough
		printf("Customer [%d] is waiting for Manager for negotiations.", myIndex);
/*		managerLock->Acquire();
		managerCV->Wait(managerLock);
		
		managerLock->Release();
		for(i=0;i<myItemCount;i++)
		{
			printf("Customer [%d] tells Manager to remove [%d] from trolly", myIndex, myItemsIDs[i]);
			needMoney -= (myItemsIDs[i]+1)*10;
			if (needMoney <= 400) {
				salesMoneyLock->Acquire();
				salesMoney += needMoney;
				salesMoneyLock->Release();
				printf("Customer [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.", myIndex, needMoney);
				
				printf("Customer [%d] got receipt from Manager and is now leaving.", myIndex);
				break;
			}
		}
 */	
	}
	
	
	//Customer returns the trolly
	findTrollyLock.Acquire();
	trollyLeft++;
	findTrollyCV.Broadcast(&findTrollyLock); 
	findTrollyLock.Release();
}

void thread_manager() {
	while (true) {
		managerLock->Acquire();
		if (managerstatus = 0) {
			<#statements#>
		}
		managerCV->Wait(managerLock);
		 
		managerLock->Release();
		 
	}
}
	
	
// --------------------------------------------------
// thread_salesman() -- thread for initializing every salesman who welcomes the customer.
// --------------------------------------------------
void thread_salesman(int myID) {
	
	int myCustomerID;
	int myIndex,myDepartment,i;
	
	myDepartment = myID / 3;
	myIndex = myID % 3;
	
/*	if(myID == 0)
	{
		myIndex=0;
		myDepartment = 0;
	}
	if(myID == 1)
	{
		myIndex=1;
		myDepartment = 0;
	}
	if(myID == 2)
	{
		myIndex=2;
		myDepartment = 0;
	}
	if(myID == 3)
	{
		myIndex=0;
		myDepartment = 1;
	}
	if(myID == 4)
	{
		myIndex=1;
		myDepartment = 1;
	}
	if(myID == 5)
	{
		myIndex=2;
		myDepartment = 1;
	}
	if(myID == 6)
	{
		myIndex=0;
		myDepartment = 2;
	}
	if(myID == 7)
	{
		myIndex=1;
		myDepartment = 2;
	}
	if(myID == 8)
	{
		myIndex=2;
		myDepartment = 2;
	}
	if(myID == 9)
	{
		myIndex=0;
		myDepartment = 3;
	}
	if(myID == 10)
	{
		myIndex=1;
		myDepartment = 3;
	}
	if(myID == 11)
	{
		myIndex=2;
		myDepartment = 3;
	}
	if(myID == 12)
	{
		myIndex=0;
		myDepartment = 4;
	}
	if(myID == 13)
	{
		myIndex=1;
		myDepartment = 4;
	}
	if(myID == 14)
	{
		myIndex=2;
		myDepartment = 4;
	}
*/	
	
    while(true)
	{
	//getting ready to welcome the first available customer.
	custWaitingLineLock[myDepartment]->Acquire();
	//printf("----------------i am salesman[%d] department[%d] and my queues is [%d] \n",myIndex+1,myDepartment+1,custWaitingLineCount[myDepartment]);
	
	//check the queue if any customer is waiting 
    if(custWaitingLineCount[myDepartment] > 0)
	{
		//printf("%d", custWaitingLineCount);
		//if yes signals the first one to walk ahead and come to him.
		//printf ("salesman %d: How are you doing today , Welcome to Ayush's super-market\n", myIndex+1);
		custWaitingLineCV[myDepartment]->Signal(custWaitingLineLock[myDepartment]);
		//decreaments the line counter.
		custWaitingLineCount[myDepartment]--;
		//sets its status to busy.
		salesmanStatus[myDepartment][myIndex] = 1;
		
		salesmanLock[myDepartment][myIndex]->Acquire();
		salesmanStatus[myDepartment][myIndex] = 3;
		custWaitingLineLock[myDepartment]->Release();
		//salesmanStatus[myIndex] = 3;
		salesmanCV[myDepartment][myIndex]->Wait(salesmanLock[myDepartment][myIndex]);
		//myCustomerID = custNumber[myIndex];
		if(custStatus[myDepartment][myIndex] == 1)
		{
			printf("DepartmentSalesman [%d] Welcomes PriviledgedCustomer [%d] to Department [%d]\n", myIndex+1, custNumber[myDepartment][myIndex]+1,myDepartment+1);
		}
		else
		{
			printf("DepartmentSalesman [%d] Welcomes Customer [%d] to Department [%d]\n", myIndex+1, custNumber[myDepartment][myIndex]+1,myDepartment+1);
		}
		salesmanCV[myDepartment][myIndex]->Signal(salesmanLock[myDepartment][myIndex]);
		
	}
	

	else
	{
		// no one there so .. status is set to free (0)
		salesmanLock[myDepartment][myIndex]->Acquire();
		printf ("salesman %d: no one is there to welcome , i am free currently line count is %d my department is [%d]\n", myIndex+1, custWaitingLineCount[myDepartment], myDepartment+1);
		salesmanStatus[myDepartment][myIndex] = 0;
		custWaitingLineLock[myDepartment]->Release();
		//salesmanLock[myIndex]->Acquire();
			
	}
	// now it waits for some customer to come or waits that the customer has successfully entered the market .. goes to sleep.
	//salesmanStatus[myIndex] = 3;
	//printf ("\ntest\n");
	salesmanCV[myDepartment][myIndex]->Wait(salesmanLock[myDepartment][myIndex]);
	//printf ("\ntest\n");
	salesmanStatus[myDepartment][myIndex] = 1;
	salesmanLock[myDepartment][myIndex]->Release();
	
	}
}


/*********************************************************************************/

	
// --------------------------------------------------
// thread_cashier() -- thread for initializing every cashier who will bill the customer.
// --------------------------------------------------

void thread_cashier(int myIndex) {
	
	while (true) {
		cashierMsgLock[myIndex]->Acquire();
		// if the cashier's status is on Break
		if (cashierStatus[myIndex] == 2) {
			printf("Cashier [%d] is going on break.", myIndex);
			if (custCashierWaitingLineCount[myIndex] > 0) {
				// inform the waiting customer to go to other line
				custCashierWaitingLineCV[myIndex]->Broadcast(custCashierWaitingLineLock[myIndex]);
			}
			custCashierWaitingLineCount[myIndex] = 0;
		}
		else if (cashierStatus[myIndex] == 3){
			printf("Cashier [%d] was called from break by Manager to work.", myIndex);
			cashierStatus[myIndex] == 0;
		}
		// wake up by customer
		else if (cashierStatus[myIndex] == 1) {
			
			// calculate the bill
			int needMoney=0;
			for (int i = 0; i < currentCasherItemsNo[myIndex]; i++) {
				currentCashierCus[myIndex];
				printf("Cashier [%d] got [%d] from trolly of Customer [%d].", myIndex, currentCashierItems[myIndex][i], currentCashierCus[myIndex]);
				needMoney += (currentCashierItems[myIndex][i]+1)*10;
			}
			printf("Cashier [%d] tells Customer [%d] total cost is $[%d].", myIndex, currentCashierCus[myIndex], needMoney);
			currentCashierMoney[myIndex] = needMoney;
			
			// inform customer he need to pay
			cashierCV[myIndex]->Signal(cashierMsgLock[myIndex]);
			
			// wait for customer to pay
			cashierCV[myIndex]->Wait(cashierMsgLock[myIndex]);
			if (needMoney > 400) {
				// inform manager
				printf("Cashier [%d] asks Customer [%d] to wait for Manager.", myIndex, currentCashierCus[myIndex]);
				
				printf("Cashier [%d] informs the Manager that Customer [%d] does not have enough money.", myIndex, currentCashierCus[myIndex]);
			}
			else {
				printf("Cashier [%d] got money $[%d] from Customer [%d].", myIndex, needMoney, currentCashierCus[myIndex]);
				counter[myCashier] += needMoney; // one customer at one cashier, there would be any conflict
			}
			
			// give customer recipe and tell him leave
			cashierCV[myIndex]->Signal(cashierMsgLock[myIndex]);
			printf("Cashier [%d] gave the receipt to Customer [%d] and tells him to leave.", myIndex, currentCashierCus[myIndex]);
			
		}		
		// wait for next wake msg
		cashierCV[myIndex]->Wait(cashierMsgLock[myIndex]);
		cashierMsgLock[myIndex]->Release();	
	}
	
//	int myCustomerID;
//	int i;
		
//    while(true)
//	{
		
	//getting ready to welcome the first available customer.
//	custCashierWaitingLineLock[myIndex]->Acquire();
	//printf("----------------i am salesman[%d] department[%d] and my queues is [%d] \n",myIndex+1,myDepartment+1,custWaitingLineCount[myDepartment]);
	//check the queue if any customer is waiting

	// check the priviledge line first
//	if (priviledgeCustWaitingLineLength > 0) {
//		int* customerIndex = priviledgeCustWaitingLine->Remove();
//		priviledgeCustWaitingLineLength--;
//		priviledgeCustWaitingLineCV->Signal(priviledgeCustWaitingLineLock);
//		priviledgeCustWaitingLineLock->Release();
//	} 
//	else if (custCashierWaitingLineCount[myIndex] > 0)
//	{
//		printf(" [%d]  says count is  [%d] \n",myIndex+1 ,custCashierWaitingLineCount[myIndex]);
//		custCashierWaitingLineCV[myIndex]->Signal(custCashierWaitingLineLock[myIndex]);
		//custCashierWaitingLineLock[myIndex]->Release();
		//if yes signals the first one to walk ahead and come to him.
		//printf ("salesman %d: How are you doing today , Welcome to Ayush's super-market\n", myIndex+1);
		/*
		custCashierWaitingLineCV[myIndex]->Signal(custWaitingLineLock[myIndex]);
		//decreaments the line counter.
		custWaitingLineCount[myDepartment]--;
		//sets its status to busy.
		salesmanStatus[myDepartment][myIndex] = 1;
		
		salesmanLock[myDepartment][myIndex]->Acquire();
		salesmanStatus[myDepartment][myIndex] = 3;
		custWaitingLineLock[myDepartment]->Release();
		//salesmanStatus[myIndex] = 3;
		salesmanCV[myDepartment][myIndex]->Wait(salesmanLock[myDepartment][myIndex]);
		//myCustomerID = custNumber[myIndex];
		if(custStatus[myDepartment][myIndex] == 1)
		{
			printf("DepartmentSalesman [%d] Welcomes PriviledgedCustomer [%d] to Department [%d]\n", myIndex+1, custNumber[myDepartment][myIndex]+1,myDepartment+1);
		}
		else
		{
			printf("DepartmentSalesman [%d] Welcomes Customer [%d] to Department [%d]\n", myIndex+1, custNumber[myDepartment][myIndex]+1,myDepartment+1);
		}
		salesmanCV[myDepartment][myIndex]->Signal(salesmanLock[myDepartment][myIndex]);
		*/
//	}
	

//	else
//	{
//		printf("i am free now\n");
		//printf(" [%d]  says count is  [%d] \n",myIndex+1 ,custCashierWaitingLineCount[myIndex]);
//		cashierStatus[myIndex]=0;
		//cashierCV[myIndex]->Wait(custCashierWaitingLineLock[myIndex]);
		//custCashierWaitingLineLock[myIndex]->Release();
		// no one there so .. status is set to free (0)
		//salesmanLock[myDepartment][myIndex]->Acquire();
		//printf ("salesman %d: no one is there to welcome , i am free currently line count is %d my department is [%d]\n", myIndex+1, custWaitingLineCount[myDepartment], myDepartment+1);
		//salesmanStatus[myDepartment][myIndex] = 0;
		//custWaitingLineLock[myDepartment]->Release();
		//salesmanLock[myIndex]->Acquire();
	}
	// now it waits for some customer to come or waits that the customer has successfully entered the market .. goes to sleep.
	//salesmanStatus[myIndex] = 3;
	//printf ("\ntest\n");
	//salesmanCV[myDepartment][myIndex]->Wait(salesmanLock[myDepartment][myIndex]);
	//printf ("\ntest\n");
	//salesmanStatus[myDepartment][myIndex] = 1;
	//salesmanLock[myDepartment][myIndex]->Release();
//	cashierCV[myIndex]->Wait(custCashierWaitingLineLock[myIndex]);
//	custCashierWaitingLineLock[myIndex]->Release();
	
//	}
}



/*********************************************************************************/










// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------



void startMenu()
{
	int choice = 13;
	int exitDecider = 0;
	
	
	system("clear");
	do 
	{
		//system("clear");
		printf("\n\t\t\t\t\t*************************************************************************\n");
		printf("\t\t\t\t\t*          Welcome to Thread Simulation for csci 402 Summer 2012        *\n");
		printf("\t\t\t\t\t*************************************************************************\n");
		printf("\n\t\t\t\t\t Please select from the below options");
		printf("\n\t\t\t\t\t ------------------------------------\n\n");
		printf("\n\t\t\t\t\t   1. \t Run the Complete Simulation");
		printf("\n\t\t\t\t\t   2. \t Run Test 1");
		printf("\n\t\t\t\t\t   3. \t Run Test 2");
		printf("\n\t\t\t\t\t   4. \t Run Test 3");
		printf("\n\t\t\t\t\t   5. \t Run Test 4");
		printf("\n\t\t\t\t\t   6. \t Run Test 5");
		printf("\n\t\t\t\t\t   7. \t Run Test 6");
		printf("\n\t\t\t\t\t   8. \t Run Test 7");
		printf("\n\t\t\t\t\t   9. \t Run Test 8");
		printf("\n\t\t\t\t\t   10. \t Run Test 9");
		printf("\n\t\t\t\t\t   11. \t Run Test 10");
		printf("\n\t\t\t\t\t   12. \t Exit the Program");
		
			
		printf("\n\n\t\t\t\t\t Enter the choice (NUMBERS ONLY) -- ");
		
		scanf ("%d", &choice );
		
		switch(choice){
		
		case 1 :
			ThreadTest();
			//exitDecider = 1;
			break;
		case 2 :
			//DisplayData();
			printf("Work in progress 2.");
			exitDecider = 1;
			break;
		case 3 :
			printf("Work in progress 3.");
			exitDecider = 1;
			break;
		case 4 :
			printf("Work in progress 4.");
			exitDecider = 1;
			break;
		case 5 :
			printf("Work in progress 5.");
			exitDecider = 1;
			break;
		case 6 :
			printf("Work in progress 6.");
			exitDecider = 1;
			break;
		case 7 :
			printf("Work in progress 7.");
			exitDecider = 1;
			break;
		case 8 :
			printf("Work in progress 8.");
			exitDecider = 1;
			break;
		case 9 :
			printf("Work in progress 9.");
			exitDecider = 1;
			break;
		case 10 :
			printf("Work in progress 10.");
			exitDecider = 1;
			break;
		case 11 :
			printf("Work in progress 11");
			exitDecider = 1;
			break;
		case 12 :
			printf("you want to exit...exiting\n");
			exitDecider = 1;
			break;
		default :
			printf("an invalid  option ...exiting\n");
			exitDecider = 1;
			break;
		}
		
				
	}while(exitDecider == 0 );
}


void DisplayData()
{
	int i,j;
	for (i=0;i<MAX_NO_DEPARTMENT ;i++)
	{
		for(j=0;j<NO_OF_ITEMS;j++)
		{
			printf(" [%d] ",itemLeft[i][j]);
			//itemPrice[i][j] = (j+1)*10;
		}
		printf("\n");
	}
}




void ThreadTest() {
    Thread *t;
    
	char *name;
    int i,j;
		
	//printf("Number of Cashiers = [%d]\n",MAX_NO_CASHIER);
	//printf("Number of Cashiers = [%d]\n",MAX_NO_CASHIER);
	//printf("Number of Cashiers = [%d]\n",MAX_NO_CASHIER);
	//printf("Number of Cashiers = [%d]\n",MAX_NO_CASHIER);
	//printf("Number of Cashiers = [%d]\n",MAX_NO_CASHIER);
	//printf("Number of Cashiers = [%d]\n",MAX_NO_CASHIER);
	//printf("\n how are you");
	//scanf("%d",&i);
		
	//intialize the total number of items left to 20 and set there price	
	for (i=0;i<MAX_NO_DEPARTMENT ;i++)
	{
		for(j=0;j<NO_OF_ITEMS;j++)
		{
			itemLeft[i][j] = 20;
			itemPrice[i][j] = (j+1)*10;
		}
	}
	
	//intialize the locks and condition varialbles as well as the salesmanStatus to busy
	for (i=0;i<MAX_NO_DEPARTMENT ;i++)
	{
		for (j=0 ; j<MAX_NO_SALESMAN; j++)
		{
		name = new char [20];
		sprintf(name,"sales_lock_%d",i);
		salesmanLock[i][j] = new Lock(name);
		sprintf(name,"sales_cv_%d",i);
		salesmanCV[i][j] = new Condition(name);
		salesmanStatus[i][j]=1;
		}
	}
	
	
	
	//intialize all  the waiting queue to 0 count and intialize there respective locks and condition variables
	for(i=0;i<MAX_NO_DEPARTMENT;i++)
	{
		name = new char [20];
		sprintf(name,"custwaitingline_lock_%d",i);
		custWaitingLineLock[i] = new Lock(name);
		sprintf(name,"custwaitingline_cv_%d",i);
		custWaitingLineCV[i] = new Condition(name);
		custWaitingLineCount[i]=0;
	}
	
	
	
	//Lock *deptShoppingLock[MAX_NO_DEPARTMENT];
	
	for(i=0;i<MAX_NO_DEPARTMENT;i++)
	{
		name = new char [20];
		sprintf(name,"deptshopping_lock_%d",i);
		deptShoppingLock[i] = new Lock(name);
		sprintf(name,"deptshopping_cv_%d",i);
		deptShoppingCV[i] = new Condition(name);
		//custWaitingLineCount[i]=0;
	}
	
	//inialising the cashier related locks, condition and count variables
	for(i=0;i<MAX_NO_CASHIER; i++)
	{
		name = new char [20];
		sprintf(name,"ccwaitline_lock_%d",i);
		custCashierWaitingLineLock[i] = new Lock(name);
		sprintf(name,"ccwaitine_cv_%d",i);
		custCashierWaitingLineCV[i] = new Condition(name);
		
		sprintf(name,"cashierwaiting_cv_%d",i);
		cashierCV[i] = new Condition(name);
		
		custCashierWaitingLineCount[i]=0;
		cashierStatus[i] = 1;
		
		sprintf(name,"cashierMsgLock_%d",i);
		cashierMsgLock[i] = new Lock(name);
		
	}
		
		
	//simluation test for cust sales
	printf("starting the simulation\n");
	
	// creating the customers 	
	for (  i = 0 ; i < 15 ; i++ )
	{
		name = new char [20];
		sprintf(name,"customer_%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)thread_customer,i);
    }
	
	// creating the salesman
	for (  i = 0 ; i < 15 ; i++ ) 
	{
		
		name = new char [20];
		sprintf(name,"salesman_%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)thread_salesman,i);
		
	}
	
	//creating the cashiers
	for (  i = 0 ; i < MAX_NO_CASHIER ; i++ ) 
	{
		
		name = new char [20];
		sprintf(name,"salesman_%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)thread_cashier,i);
		
	}
	
	
	
}
#endif
