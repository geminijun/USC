
#include "copyright.h"
#include "system.h"


/*----------------------------------------------------
 Global Variables declaration and definition
 ----------------------------------------------------*/
Lock* lTrolly;
Condition* cTrolly;
int num_trolly;

Lock* lSalesmen[NUM_DEPARTMENT];
bool isSalesmanBusy[NUM_DEPARTMENT][NUM_SALESMEN_PER_DEP];
Condition* cSalesmen[NUM_DEPARTMENT];


Lock* items_lock[NUM_DEPARTMENT];
int num_items[NUM_DEPARTMENT];

Condition* cLoadItem[NUM_DEPARTMENT];

Lock* lCashiers;
bool isCashierOnWork[NUM_CASHIER];

Lock* lCashier[NUM_CASHIER];
int lineLength[NUM_CASHIER];
List* cashierList[NUM_CASHIER];

Condition* cCashier[NUM_CASHIER];

Lock* lManager;

int cashierSale[NUM_CASHIER];
int totalSale;

int needMoney[NUM_CUSTOMER];