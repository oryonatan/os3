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
#include <memory>

#include <iostream>

using namespace std;

//The Task struct implements a task to be printed to the disk

struct Task {
	//The ID of the struct given by the TaskList (the lowes free ID)
	int id;
	//The buffer to be printed
	shared_ptr<string> data;
	//The conditional variable used to signify the task was printed
	pthread_cond_t sig;
	//The mutex associated with the conditional variable
	pthread_mutex_t mut;

	//Constructor
	Task(int id, shared_ptr<string> instring) :
			id(id), data(instring) {
		pthread_cond_init(&sig ,NULL);
		pthread_mutex_init(&mut,NULL);
	}
	;
	//Destructor
	~Task() {
//		cout<< "Deleting task\n"; //DEBUG
		pthread_cond_broadcast(&sig);
		pthread_cond_destroy(&sig);
		pthread_mutex_destroy(&mut);
	}
	;
};

//Describes a position of a written task whether it was written, still waits to be written, or not found
enum location{
	RUNNING,HISTORY,NOT_FOUND
};


//The TaskList class includes all the tasks that are, were, or will be written by outputdevice.
// It controls both the printed queue for the tasks to be written, a history of all the written
//tasks, and enables to access to the tasks by their ID.
//The TaskList is thread safe and protects its inner data structures by mutexes.
class TaskList {
public:
	TaskList();
	~TaskList();
	//Add a task to the tail of the task queue
	int addTask(shared_ptr<string>);
	//Returns the head of the printing queue (without popping it)
	shared_ptr<Task> front() const ;
	//Pop the head of the printing queue
	int popTask();
	//Searches the data structures for the printing task with the given tid,
	//and returns whether the task is found in the printing queue or in history
	location findTid(int tid);
	//returns the mutex associated with the task with given tid
	pthread_mutex_t* getSignalMutex(int tid) const;
	//returns the conditional variable associate with the task with the given tid
	pthread_cond_t* getSignal(int tid) const;
private:
	//Get the lowest ID that is not in the printing queue
	int getFreeID();
	//delete all the tasks
	void deleteAllTasks();

	//the printing queue
	queue<shared_ptr<Task>> tasks;
	//the structure that enables fetching the task by id
	map <int,shared_ptr<Task>> ids;
	//history of all the tasks that were printed to the device.
	set<int> history;
	//the mutex that protects the data structures
	pthread_mutex_t listMutex;
};

#endif /* TaskList_H_ */



