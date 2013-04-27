/*
 * TaskList.h
 *
 *  Created on: Apr 24, 2013
 *      Author: maria
 */

#ifndef TASKLIST_H_
#define TASKLIST_H_

#include <vector>
#include "pthread.h"
#include "printTask.h"

class TaskList {
public:
	TaskList();
	virtual ~TaskList();
	//returns the lowest ID in the list

	bool isEmpty();
	int size();
	void addTask (int tid);
	int getFreeID();
	printTask& operator [] (int tid);
	bool wasItWritten (int tid);

private:
	pthread_mutex_t listMutex;
	std::vector<int> tasks;
};

#endif /* TaskList_H_ */
