/*
 * outputdevice.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: maria
 */

#include <iostream>
#include <cstdio>
#include "pthread.h"
#include <algorithm>

#include "outputdevice.h"
#include "IDQueue.h"
#include "printTask.h"
#include "exceptions.h"
#include "TaskList.h"
#include "wrappedFunctions.h"

#define LIB_ERROR "Output device library error\n"
#define SYS_ERROR "system error\n"


#define YES 0
#define NO 1

#define SUCCESS 0
#define FILESYSTEM_ERROR -2
#define TID_NOT_FOUND_ERROR -2
#define WAITFORCLOSE_SUCCESSFUL 1
#define FAIL -1
#define APPEND "a"

using namespace std;

pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t addTask;
pthread_mutex_t writeMutex;
pthread_t daemonThread;

bool closing = true;
bool initialized = false;
TaskList* allTasks = NULL;
IDQueue* printQueue = NULL;
FILE* diskFile = NULL;
int printCounter = 0;

//frees the memory in case of shutdown caused by exception
void closeEverything()
{
	//TODO - destroy mutexes?
	if (printQueue != NULL)
	{
		delete printQueue;
	}

	if (allTasks != NULL)
	{
		delete allTasks;
	}

	if (diskFile != NULL)
	{
		fclose(diskFile);
	}
	initialized = false;
	printCounter = 0;
}

//The entry point to the writing daemon thread
void* writingFunc(void *)
{
	return NULL;
}

int initdevice(char *filename)
{
	pthreadMutexLock(&initMutex);
	if (initialized || filename == NULL)
	{
		closeEverything();
		pthread_mutex_unlock(&initMutex);
		return FAIL;
	}

	if ((diskFile = fopen(filename, APPEND)) == NULL)
	{
		closeEverything();
		pthread_mutex_unlock(&initMutex);
		return FILESYSTEM_ERROR;
	}

	printQueue = new IDQueue();
	allTasks = new TaskList();
//	pthread_create(&daemonThread, NULL, writingFunc, NULL);
	pthread_mutex_init(&addTask, NULL);
	pthreadMutexUnlock(&initMutex);
	initialized = true;
	return SUCCESS;
}

int write2device(char *buffer, int length)
{
	int newId;
	//initdevice must be called before calling any function
	if (!initialized || closing)
	{
		return FAIL;
	}
	pthreadMutexLock(&addTask);
	char * data = (char*) malloc(length);
	newId = allTasks->addTask(data);
	pthreadMutexUnlock(&addTask);
	return newId;
}

int flush2device(int task_id)
{
	if (!initialized || task_id < 0)
	{
		return FAIL;
	}
	location loc = allTasks->findTid(task_id);
	switch (loc)
	{
	case NOT_FOUND:
		return FAIL;
	case HISTORY:
		return OKAY;
	case RUNNING:
		pthread_cond_wait(allTasks->getSignal(task_id),
				allTasks->getSignalMutex(task_id));
		return OKAY;

	}
	return FAIL;// unreachable , but eclipse is bugging me
	//else waitforsignal

//	try
//	{
//		if (!initialized)
//		{
//			throw LibraryErrorException();
//		}
//
//		if (task_id < 0 || task_id > allTasks->size())
//		{
//			throw LibraryErrorException();
//		}
//		//TODO not finished
//	}
//	catch ( TidNotFoundException& e)
//	{
//		//TODO print error message
//		return TID_NOT_FOUND_ERROR;
//	}
//	catch (exception& e)
//	{
//		//TODO print error message
//		return FAIL;
//	}
}

int wasItWritten(int task_id)
{
	switch(allTasks->findTid(task_id))
	{
	case RUNNING:
		return NO;
	case HISTORY:
		return YES;
	case NOT_FOUND:
		return TID_NOT_FOUND_ERROR;
	}
	return FAIL;// unreachable , but eclipse is bugging me
}

int howManyWritten()
{
	if (closing)
	{
		return FAIL;
	}
	return printCounter;
}

void closedevice()
{
	closing = true;
}

int wait4close()
{
	if (!closing)
	{
		return FAIL;
	}
	return WAITFORCLOSE_SUCCESSFUL;
}

