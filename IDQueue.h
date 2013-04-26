/*
 * IDQueue.h
 *
 *  Created on: Apr 24, 2013
 *      Author: maria
 */

#ifndef IDQueue_H_
#define IDQueue_H_

#include <queue>
#include "printTask.h"
#include "pthread.h"

class IDQueue {
public:
	IDQueue();
	virtual ~IDQueue();
	int pop();
	void push (int newID);

private:
	pthread_mutex_t queueMutex;
	std::queue<int> tidQueue;

};

#endif /* IDQueue_H_ */
