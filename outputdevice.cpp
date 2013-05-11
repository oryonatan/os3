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
#include <cstring>
#include <sstream>
#include "outputdevice.h"
#include "TaskList.h"
#include "errno.h"
#include <fstream>
#include "limits.h"
#include "safelocks.h"

#define LIB_ERROR_MESSAGE "Output device library error"
#define SYS_ERROR_MESSAGE "system error"

#define YES 0
#define NO 1

//constant return values as determined in Ex3
#define SUCCESS 0
#define FILESYSTEM_ERROR -2
#define TID_NOT_FOUND_ERROR -2
#define WAITFORCLOSE_SUCCESSFUL 1
#define FLUSH_SUCCESFUL 1
#define FAIL -1
#define WAIT_FOR_CLOSE_TO_EARLY -2

#define APPEND "a"

using namespace std;

pthread_mutex_t printCounterMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t closeMutex = PTHREAD_MUTEX_INITIALIZER;

//The mutex that makes sure only one initialization process goes at the same time
pthread_mutex_t initMutex;

//The daemon thread that does all the printing
pthread_t daemonThread;

//Signifies if the device is currently closing
bool closing = false;
//Signifies whether an initializing function was called succesfully
bool initialized = false;
unique_ptr<TaskList> allTasks;
ofstream diskFile;
long printCounter = 0;

//Helper function that destroys all the resources and frees the memory
void closeEverything()
{
	try
	{
		safeLock(&initMutex);
		if (diskFile != NULL)
		{
			diskFile.close();
		}

		initialized = false;
		safeUnlock(&initMutex);
		safeMutexDestroy(&closeMutex);
		safeMutexDestroy(&initMutex);
		safeMutexDestroy(&printCounterMut);

	} catch (PthreadError &e)
	{
		cerr << SYS_ERROR_MESSAGE << endl;
	}
}

//The entry point to the writing daemon thread
void* writingFunc(void *)
{
	try
	{
		shared_ptr<Task> firstTask;
		safeLock(&initMutex);
		while (initialized)
		{
			safeUnlock(&initMutex);
			if ((firstTask = allTasks->popTask()) != NULL)
			{

				diskFile.write((char*) &(firstTask->data)[0],
						firstTask->length);
				if (diskFile.fail())
				{
					cerr << SYS_ERROR_MESSAGE << endl;
					exit(-2);
				}
				diskFile.flush();
				safeLock(&printCounterMut);
				printCounter++;
				allTasks->done(firstTask->id);
				safeBroadCast(&(firstTask->sig));
				safeUnlock(&printCounterMut);
			}
			else
			{
				safeLock(&closeMutex);
				if (closing)
				{
					closing = false;
					safeUnlock(&closeMutex);
					closeEverything();
					return NULL;
				}
				safeUnlock(&closeMutex);
			}

			safeLock(&initMutex);
		}
		return NULL;
	} catch (PthreadError &e)
	{
		closeEverything();
		cerr << SYS_ERROR_MESSAGE << endl;
		exit(-2);
	}
}

int initdevice(char *filename)
{
	try
	{
		safeLock(&initMutex);
		allTasks = unique_ptr<TaskList>(new TaskList());
		printCounter = 0;
		if (initialized || filename == NULL)
		{
			safeUnlock(&initMutex);
			return FAIL;
		}

		diskFile.open(filename, ofstream::app | ofstream::binary);
		if (!diskFile.is_open())
		{
			safeUnlock(&initMutex);
			cerr << SYS_ERROR_MESSAGE << endl;
			return FILESYSTEM_ERROR;
		}

		if (pthread_create(&daemonThread, NULL, writingFunc, NULL))
		{
			safeUnlock(&initMutex);
			cerr << SYS_ERROR_MESSAGE << endl;
			return FAIL;

		}

		initialized = true;
		safeUnlock(&initMutex);
		return SUCCESS;
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(initMutex);
		cerr << SYS_ERROR_MESSAGE << endl;
		return FAIL;
	}
}

int write2device(char *buffer, int length)
{
	int newId;
	try
	{
		safeLock(&closeMutex);

		safeLock(&initMutex);
		if (!initialized || closing)
		{
			safeUnlock(&closeMutex);
			safeUnlock(&initMutex);
			cerr << LIB_ERROR_MESSAGE << endl;
			return FAIL;
		}
		safeUnlock(&closeMutex);
		safeUnlock(&initMutex);
		vector<char> data(buffer, buffer + length);
		newId = allTasks->addTask(data, length);
		return newId;
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(closeMutex);
		UNLOCK_IGNORE(closeMutex);
		cerr << SYS_ERROR_MESSAGE << endl;
		return FAIL;
	}

}

int flush2device(int task_id)
{
	try
	{
		if (task_id < 0)
		{
			cerr << LIB_ERROR_MESSAGE << endl;
			return TID_NOT_FOUND_ERROR;
		}
		safeLock(&initMutex);
		if (!initialized)
		{
			cerr << LIB_ERROR_MESSAGE << endl;
			safeUnlock(&initMutex);
			return FAIL;
		}
		safeUnlock(&initMutex);
		int status = allTasks->waitToEnd(task_id);
		if (status == TID_NOT_FOUND_ERROR)
		{
			cerr << LIB_ERROR_MESSAGE << endl;
		}
		return status;
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(initMutex);
		cerr << SYS_ERROR_MESSAGE << endl;
		return FAIL;
	}

}

int wasItWritten(int task_id)
{
	switch (allTasks->findTid(task_id))
	{
	case RUNNING:
		return NO;
	case HISTORY:
		return YES;
	case NOT_FOUND:
		cerr << LIB_ERROR_MESSAGE << endl;
		return TID_NOT_FOUND_ERROR;
	}
	return FAIL; // unreachable , but eclipse is bugging me
}

int howManyWritten()
{
	try
	{
		safeLock(&initMutex);
		if (!initialized)
		{
			safeUnlock(&initMutex);
			return FAIL;
		}
		safeUnlock(&initMutex);
		safeLock(&printCounterMut);
		int pc = printCounter;
		if (printCounter > INT_MAX)
		{
			safeUnlock(&printCounterMut);
			return INT_MIN;
		}
		safeUnlock(&printCounterMut);
		return pc;
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(initMutex);
		UNLOCK_IGNORE(printCounterMut);
		cerr << SYS_ERROR_MESSAGE << endl;
		return FAIL;
	}
}

void closedevice()
{
	try
	{
		safeLock(&closeMutex);
		closing = true;
		safeUnlock(&closeMutex);
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(closeMutex);
		cerr << SYS_ERROR_MESSAGE << endl;
	}
}

int wait4close()
{
	try
	{

		void* ret = NULL;
		safeLock(&closeMutex);
		safeLock(&initMutex);
		if (!(closing && initialized))
		{
			safeUnlock(&closeMutex);
			safeUnlock(&initMutex);
			cerr << LIB_ERROR_MESSAGE << endl;
			return WAIT_FOR_CLOSE_TO_EARLY;
		}
		safeUnlock(&initMutex);
		safeUnlock(&closeMutex);
		safeJoin(daemonThread, &ret);
		safeLock(&closeMutex);
		closing = false;
		safeUnlock(&closeMutex);
		return WAITFORCLOSE_SUCCESSFUL;
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(closeMutex);
		UNLOCK_IGNORE(initMutex);
		cerr << SYS_ERROR_MESSAGE << endl;
		return FAIL;
	}

}

