/*
 * TaskList.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: maria
 */

#include "TaskList.h"
#include <limits.h>
int TaskList::addTask(char* data) {
	int tid = this->getFreeID();
	if (tid == FAIL) {
		return FAIL;
	}
	tasks.push(Task(tid, data));
	return OKAY;
}

TaskList::TaskList() :
		tasks() {
}

TaskList::~TaskList() {

	queue<Task> empty;
	swap(this->tasks, empty);
}

int TaskList::getFreeID() {
	for (int i = 0; i <= INT_MAX; ++i) {
		if (ids.find(i) == ids.end()) {
			return i;
		}
	}
	return FAIL;
}

Task TaskList::popTask(){
	Task ret = tasks.front();
	tasks.pop();
	ids.erase(ids.find(ret.id));
	return ret;
}
