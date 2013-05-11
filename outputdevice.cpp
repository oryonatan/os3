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
//initmutex will never be erased , the rest are destroyed on close
pthread_mutex_t printCounterMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t closeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t initMutex =PTHREAD_MUTEX_INITIALIZER;

//The daemon thread that does all the printing
pthread_t daemonThread;

//Signifies if the device is currently closing
bool closing = false;
//Signifies whether an initializing function was called succesfully
bool initialized = false;
unique_ptr<TaskList> allTasks;
ofstream diskFile;
long printCounter = 0;
bool finishedClosing = false;

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
			//If list is not empty - print
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
			//if list is empty - should I close?
			else
			{
				safeLock(&closeMutex);
				if (closing)
				{
					closing = false;
					safeUnlock(&closeMutex);
					closeEverything();
					safeLock(&initMutex);
					finishedClosing =true;
					safeUnlock(&initMutex);
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
		finishedClosing = false;
		allTasks = unique_ptr<TaskList>(new TaskList());
		printCounter = 0;
		if (initialized || filename == NULL)
		{
			safeUnlock(&initMutex);
			return FAIL;
		}
		//open file
		diskFile.open(filename, ofstream::app | ofstream::binary);
		if (!diskFile.is_open())
		{
			safeUnlock(&initMutex);
			cerr << SYS_ERROR_MESSAGE << endl;
			return FILESYSTEM_ERROR;
		}
		//create writer thread
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
		//can't write if not initialized or if closing
		if (!initialized || closing)
		{
			safeUnlock(&closeMutex);
			safeUnlock(&initMutex);
			cerr << LIB_ERROR_MESSAGE << endl;
			return FAIL;
		}
		safeUnlock(&closeMutex);
		safeUnlock(&initMutex);
		//copy data to vector - so it can be freed
		vector<char> data(buffer, buffer + length);
		//add new task
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
		//taskid <0 is illegal
		if (task_id < 0)
		{
			cerr << LIB_ERROR_MESSAGE << endl;
			return TID_NOT_FOUND_ERROR;
		}
		safeLock(&initMutex);
		//must be initialized
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
	return FAIL; // unreachable , but eclipse insist return after switch case
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
		//create copy
		int pc = printCounter;
		if (pc > INT_MAX)
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
	// The closedevice function actually does nothing , and just set the flag for the thread to
	// close on finish and to make it impossible for others to add tasks
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
		safeLock(&initMutex);
		if (finishedClosing)//allready finished
		{
			safeUnlock(&initMutex);
			return WAITFORCLOSE_SUCCESSFUL;
		}
		safeLock(&closeMutex);
		if (!(closing && initialized))
		{
			safeUnlock(&initMutex);
			safeUnlock(&closeMutex);
			cerr << LIB_ERROR_MESSAGE << endl;
			return WAIT_FOR_CLOSE_TO_EARLY;
		}
		safeUnlock(&closeMutex);
		safeUnlock(&initMutex);
		void* ret = NULL;//we dont care for that
		safeJoin(daemonThread, &ret);
		return WAITFORCLOSE_SUCCESSFUL;
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(closeMutex);
		UNLOCK_IGNORE(initMutex);
		cerr << SYS_ERROR_MESSAGE << endl;
		return FAIL;
	}

}

