#include "synch.h"
#include "system.h"
#include "customer.h"

int getAvailableSalesman(int iDpt)
{
	int ret = -1;
	for (int i = 0; i < NUM_SALESMEN_PER_DEP; i++) {
		if (!isSalesmanBusy[iDpt][i]) {
			ret = i;
			break;
		}
	}
	return ret;
}

int getCashier()
{
	int ret = -1;
	for (int i = 0; i < NUM_CASHIER; i++) {
		if (isCashierOnWork[i]) {
			ret = i;
			break;
		}
	}
	return ret;
}

int getShortestLine()
{
	int ret = -1;
	int minLen = NUM_CUSTOMER;
	for (int i = 0; i < NUM_CASHIER; i++) {
		if (!isCashierOnWork[i]) {
			if (minLen > lineLength[i]) {
				ret = i;
				minLen = lineLength;
			}
		}
	}
	return ret;
}

void customer_start(int arg)
{
	Customer * cus = (Customer *)arg;
	printf("Customer [%d] enters the SuperMarket\n", arg->id);
	
	int itemNum = rand() % 10;  // random generate the num of item
	printf("Customer [%d] wants to buy [%d] no.of items\n", arg->id, itemNum);

	// Get a trolly
	printf("Customer [%d] gets in line for a trolly\n", arg->id);
	lTrolly->Acquire();	
	if (num_trolly == 0) {
		cTrolly->wait(lTrolly);
	}	
	num_trolly--;
	lTrolly->Release();
	printf("Customer [%d] has a trolly for shopping\n", arg->id);
	
	for (int i = 0; i < itemNum; i++) {
		// Random select a department
		int iDpt = rand() % NUM_DEPARTMENT;
		printf("Customer [%d] wants to shop in Department[%d]\n", arg->id, iDpt);

		// wait until get a salesman
		int iSalesman;
		while (1) {
			lSalesmen[iDpt]->Acquire();
			iSalesman = getAvailableSalesman(iDpt);
			if (iSalesman == -1) {
				// no available salesman
				cSalesmen[iDpt]->wait(lSalesmen[iDpt]);
			} else {
				isSalesmanBusy[iDpt][iSalesman] = true;
				lSalesmen[iDpt]->Release();
				break;
			}
		}		

		// wait until there has the item
		while (1) {
			items_lock[iDpt]->Acquire();
			if (num_items[iDpt] == 0) {
				// tell Salesman
				cLoadItem[iDpt]->Signal(items_lock[iDpt]);
				
				// realse Salesman
				isSalesmanBusy[iDpt][iSalesman] = false;
				lSalesmen[iDpt]->Release();
				
				items_lock[iDpt]->Release();
				// wait for item
				
			} else {
				// pick the item;
				num_items[iDpt]--;
				Item* item = new Item;
				cus->itemList->Append(item);
				
				// realse Salesman
				isSalesmanBusy[iDpt][iSalesman] = false;
				lSalesmen[iDpt]->Release();
				
				items_lock[iDpt]->Release();
				break;
			}
		}
	}

	
	// move to cashier
	int iCashier;
	while (1) {
		lCashiers->Acquire();
		if(cus->isPrivileged()) {
			iCashier = getCashier();
			if (iCashier == -1) {
				lCashiers->Release();
			} else {
				lCashier[iCashier]->Acquire();
				lineLength[iCashier]++;
				cashierList[iCashier]->Prepend(cus);
				cCashier[iCashier]->wait(lCashier[iCashier]);
				break;
			}
		} else {
			iCashier = getShortestLine();
			if (iCashier == -1) {
				lCashier->Release();
			} else {
				lCashier[iCashier]->Acquire();
				lineLength[iCashier]++;
				cashierList[iCashier]->Append(cus);
				cCashier[iCashier]->wait(lCashier[iCashier]);
				break;
			}
		}
	}
	
	// wait for scan
	
	if (needMoney[cus->id] > cus->hasMoney) {
		
		// leave cashier go direct to manager
		lCashier[iCashier]->Release();
		
		lManager->Acquire();
		// reduce item until has enough money
		while (1) {
			Item* item = cus->itemList->Remove();
			if (item == NULL) {
				needMoney[cus->id] = 0;
				break;
			}
			needMoney[cus->id] -= item->price;
			if (needMoney[cus->id] <= cus->hasMoney) {
				break;
			}
		}
		totalSale += needMoney[cus->id];
		lManager->Release();
		
	} else {
		
		cashierSale[iCashier] += needMoney[cus->id];

		//waitForReceipt();

		lCashier[iCashier]->Release();
	}
	
	// returen the trolly and leave
	lTrolly->Acquire();	
	num_trolly++;
	lTrolly->Release();
	
}

