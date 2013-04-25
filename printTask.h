/*
 * printTask.h
 *
 *  Created on: Apr 25, 2013
 *      Author: maria
 */

#ifndef PRINTTASK_H_
#define PRINTTASK_H_

#include "pthread.h"

class printTask {
public:
	printTask(int tid, char* buffer, int length);
	virtual ~printTask();

private:
	pthread_mutex_t taskMutex;
	int id;
	char* buffer;
};

#endif /* PRINTTASK_H_ */
