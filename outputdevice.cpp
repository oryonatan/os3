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
	pthread_mutex_lock(&initMutex);
	if (diskFile != NULL)
	{
		diskFile.close();
	}

	initialized = false;
	pthread_mutex_unlock(&initMutex);
}

//The entry point to the writing daemon thread
void* writingFunc(void *)
{
	shared_ptr<Task> firstTask;
	pthread_mutex_lock(&initMutex);
	while (initialized)
	{
		pthread_mutex_unlock(&initMutex);
		if ((firstTask  = allTasks->popTask()) != NULL)
		{

			diskFile.write(&(firstTask->data)[0],firstTask->length);
			if(diskFile.fail())
			{
				cerr << SYS_ERROR_MESSAGE << endl;
				exit (-2);
			}
			pthread_mutex_lock(&printCounterMut);
			printCounter++;
			allTasks->done(firstTask->id);
			pthread_cond_broadcast(&(firstTask->sig));
			pthread_mutex_unlock(&printCounterMut);
		}
		else
		{
			pthread_mutex_lock(&closeMutex);
			if (closing)
			{
				closeEverything();
				closing = false;
				pthread_mutex_unlock(&closeMutex);
				return NULL;
			}
			pthread_mutex_unlock(&closeMutex);
		}

		pthread_mutex_lock(&initMutex);
	}
	return NULL;
}

int initdevice(char *filename)
{
	if (pthread_mutex_lock(&initMutex))
	{
		cerr << SYS_ERROR_MESSAGE << endl;
		return FAIL;
	}
	allTasks= unique_ptr<TaskList>(new TaskList());
	printCounter = 0;
	if (initialized || filename == NULL)
	{
		pthread_mutex_unlock(&initMutex);
		cerr << LIB_ERROR_MESSAGE << endl;
		return FAIL;
	}

	diskFile.open(filename,ofstream::app|ofstream::binary);
	if (!diskFile.is_open())
	{
		pthread_mutex_unlock(&initMutex);
		cerr << SYS_ERROR_MESSAGE << endl;
		return FILESYSTEM_ERROR;
	}

	if (pthread_create(&daemonThread, NULL, writingFunc, NULL))
	{
		cerr << SYS_ERROR_MESSAGE << endl;
		return FAIL;

	}
	initialized = true;
	if (pthread_mutex_unlock(&initMutex))
	{
		cerr << SYS_ERROR_MESSAGE << endl;
		return FAIL;
	}
	return SUCCESS;
}

int write2device(char *buffer, int length)
{
	int newId;
	//initdevice must be called before calling any function
	pthread_mutex_lock(&closeMutex);
	pthread_mutex_lock(&initMutex);
	if (!initialized || closing)
	{
		pthread_mutex_unlock(&closeMutex);
		pthread_mutex_unlock(&initMutex);
		cerr << LIB_ERROR_MESSAGE << endl;
		return FAIL;
	}
	pthread_mutex_unlock(&closeMutex);
	pthread_mutex_unlock(&initMutex);
	vector<char> data(buffer,buffer+length);
	newId = allTasks->addTask(data,length);
	return newId;
}

int flush2device(int task_id)
{
	if (task_id <0 )
	{
		cerr << LIB_ERROR_MESSAGE << endl;
		return TID_NOT_FOUND_ERROR;
	}
	pthread_mutex_lock(&initMutex);
	if (!initialized)
	{
		pthread_mutex_unlock(&initMutex);
		cerr << LIB_ERROR_MESSAGE << endl;
		return FAIL;
	}
	pthread_mutex_unlock(&initMutex);
	int status = allTasks->waitToEnd(task_id);
	if (status  == TID_NOT_FOUND_ERROR)
	{
		cerr << LIB_ERROR_MESSAGE << endl;
	}
	return status;
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
	pthread_mutex_lock(&initMutex);
	if (!initialized)
	{
		pthread_mutex_unlock(&initMutex);
		return FAIL;
	}
	pthread_mutex_unlock(&initMutex);
	pthread_mutex_lock(&printCounterMut);
	int pc = printCounter;
	if (printCounter > INT_MAX)
	{
		pthread_mutex_unlock(&printCounterMut);
		return INT_MIN;
	}
	pthread_mutex_unlock(&printCounterMut);
	return pc;
}

void closedevice()
{
	pthread_mutex_lock(&closeMutex);
	closing = true;
	pthread_mutex_unlock(&closeMutex);
}

int wait4close()
{
	void* ret = NULL;
	pthread_mutex_lock(&closeMutex);
	pthread_mutex_lock(&initMutex);
	if (!(closing && initialized))
	{
		pthread_mutex_unlock(&closeMutex);
		pthread_mutex_unlock(&initMutex);
		cerr << LIB_ERROR_MESSAGE << endl;
		return WAIT_FOR_CLOSE_TO_EARLY;
	}
	pthread_mutex_unlock(&initMutex);
	pthread_mutex_unlock(&closeMutex);
	if(pthread_join(daemonThread,&ret))
	{
		cerr << SYS_ERROR_MESSAGE << endl;
		return FAIL;
	}
	pthread_mutex_lock(&closeMutex);
	closing = false;
	pthread_mutex_unlock(&closeMutex);
	return WAITFORCLOSE_SUCCESSFUL;
}

