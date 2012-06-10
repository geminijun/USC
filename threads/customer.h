#ifndef CUSTOMER_H
#define CUSTOMER_H

struct Item {
	int iDpt;
	int price;
}

struct Customer {
	int id;
	bool isPrivileged;
	int hasMoney;
	List* itemList;
};

// 
void customer_start(int id);

#endif