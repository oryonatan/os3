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
#include <map>
#include "pthread.h"
#include <malloc.h>

#include <iostream>

using namespace std;

struct Task {
	int id;
	char * data;
	pthread_cond_t sig;
	pthread_mutex_t mut;
	Task(int id, char* data) :
			id(id), data(data) {
		pthread_cond_init(&sig ,NULL);
		pthread_mutex_init(&mut,NULL);
	}
	;
	~Task() {
		cout<< "Deleting task\n"; //DEBUG
		free(data);
		pthread_cond_broadcast(&sig);
		pthread_cond_destroy(&sig);
		pthread_mutex_destroy(&mut);
	}
	;
};
enum location{
	RUNNING,HISTORY,NOT_FOUND
};

class TaskList {
	//Tasklist is not thread safe , all operations should be used
	// under mutexes
public:
	TaskList();
	~TaskList();
	int addTask(char * );
	Task * front() const ;
	int popTask();
	location findTid(int tid) const;
	pthread_mutex_t* getSignalMutex(int tid) const;
	pthread_cond_t* getSignal(int tid) const;
private:
	int getFreeID();
	void delteAllTasks();

	queue<Task *> tasks;
	map <int,Task *> ids;
	set<int> history;
};

#endif /* TaskList_H_ */



