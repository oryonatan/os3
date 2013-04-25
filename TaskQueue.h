/*
 * TaskQueue.h
 *
 *  Created on: Apr 24, 2013
 *      Author: maria
 */

#ifndef TASKQUEUE_H_
#define TASKQUEUE_H_

#include <queue>;
#include "pthread.h"

class TaskQueue {
public:
	TaskQueue();
	virtual ~TaskQueue();
	printTask pop();
	void push (printTask newTask);

private:
	pthread_mutex_t queueMutex;
	std::queue<printTask> tidQueue;

};

#endif /* TASKQUEUE_H_ */
