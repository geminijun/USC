#include "synch.h"
#include "system.h"
#include "goodsloader.h"

void goodsloader_start(int id)
{
	while(1) {
		// check if called by salesman
		
		while (1) {
			// retrieve item
			
			int time = rand() % 20 + 20;
			// put it on the store shelves
			
			if (enough()) {
				break;
			}
		}
		
	}
}
