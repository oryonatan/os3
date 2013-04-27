/*
 * TaskList.h
 *
 *  Created on: Apr 24, 2013
 *      Author: maria
 */

#ifndef TASKLIST_H_
#define TASKLIST_H_

#define OKAY 0
#define FAIL -1

#include <queue>
#include <set>
#include "pthread.h"
#include "printTask.h"
#include <malloc.h>
using namespace std;

struct Task {
	int id;
	char * data;
	Task(int id, char* data) :
			id(id), data(data) {
	}
	;
	~Task() {
		free(data);
	}
	;
};

class TaskList {
	//Tasklist is not thread safe , all operations should be used
	// under mutexes
public:
	TaskList();
	~TaskList();
	int addTask(char * );
	bool wasItWritten(int tid);
	Task popTask();
private:
	int getFreeID();
	queue<Task> tasks;
	set<int> ids;
};

#endif /* TaskList_H_ */


