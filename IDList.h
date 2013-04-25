/*
 * IDList.h
 *
 *  Created on: Apr 24, 2013
 *      Author: maria
 */

#ifndef IDLIST_H_
#define IDLIST_H_

#include <list>
#include "pthread.h"

class IDList {
public:
	IDList();
	virtual ~IDList();
	//returns the lowest ID in the list
	int lowestID();
	//returns the highest ID in the list
	int highestID();
	//checks if the list is empty
	bool isEmpty();

private:
	pthread_mutex_t listMutex;
	std:: list<int> tidList;
};

#endif /* IDLIST_H_ */
