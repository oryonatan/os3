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

//The mutex that makes sure only one initialization process goes at the same time
pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;
//The daemon thread that does all the printing
pthread_t daemonThread;

//Signifies if the device is currently closing
bool closing = false;
//Signifies whether an initializing function was called succesfully
bool initialized = false;
unique_ptr<TaskList> allTasks;
ofstream diskFile;
int printCounter = 0;

int lockAndInit(pthread_mutex_t * mut)
{
	int lockStatus =  pthread_mutex_lock(mut) ;
	if (lockStatus == EINVAL)
	{
		pthread_mutex_init(mut,NULL);
		return pthread_mutex_lock(mut);
	}
	return lockStatus;
}

//Helper function that destroys all the resources and frees the memory
void closeEverything()
{
	if (diskFile != NULL)
	{
		diskFile.close();
	}
	initialized = false;
	pthread_mutex_destroy(&initMutex);
}

//The entry point to the writing daemon thread
void* writingFunc(void *)
{
	//cerr<<"writing\n";//debug
	shared_ptr<Task> firstTask;

	while (initialized)
	{

		if (allTasks->idsLeft())
		{
//			cout<<"wrote something\n " << strlen(firstTask->data->c_str()) <<endl;//debug
			firstTask = allTasks->front();
			diskFile.write((char * const)firstTask->data.get(),firstTask->length);
			printCounter++;
			allTasks->done(firstTask->id);
			allTasks->popTask();
			pthread_cond_broadcast(&(firstTask->sig));

		}
		else if (closing)
		{
			cout << howManyWritten() << "was written" << endl;
			cout << allTasks->idsLeft() << "ids left; " << endl;
			closeEverything();
			return NULL;
		}
	}
	return NULL;
}

int initdevice(char *filename)
{
	if (lockAndInit(&initMutex))
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
	if (pthread_mutex_unlock(&initMutex))
	{
		cerr << SYS_ERROR_MESSAGE << endl;
		return FAIL;
	}
	initialized = true;
	return SUCCESS;
}

int write2device(char *buffer, int length)
{
	int newId;
	//initdevice must be called before calling any function
	if (!initialized || closing)
	{
		cerr << LIB_ERROR_MESSAGE << endl;
		return FAIL;
	}
	shared_ptr<char> data(new char[length]);
	strncpy(data.get(),buffer,length);
	newId = allTasks->addTask(data,length);
	//cout << "added task"<<endl ;//debug
	return newId;
}

int flush2device(int task_id)
{
	if (task_id <0 )
	{
		cerr << LIB_ERROR_MESSAGE << endl;
		return TID_NOT_FOUND_ERROR;
	}
	if (!initialized)
	{
		cerr << LIB_ERROR_MESSAGE << endl;
		return FAIL;
	}
	location loc = allTasks->findTid(task_id);
	switch (loc)
	{
	case NOT_FOUND:
		cerr << LIB_ERROR_MESSAGE << endl;
		return TID_NOT_FOUND_ERROR;
	case HISTORY:
		return FLUSH_SUCCESFUL;
	case RUNNING:
		lockAndInit(allTasks->getSignalMutex(task_id));
		pthread_cond_wait(allTasks->getSignal(task_id),
				allTasks->getSignalMutex(task_id));
		return FLUSH_SUCCESFUL;
	}
	return FAIL; // unreachable , but eclipse is bugging me

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
	if (!initialized)
	{
		return FAIL;
	}
	return printCounter;
}

void closedevice()
{
	//TODO - is there a possibility of error we're missing? The one that should cause exit....
	closing = true;
}

int wait4close()
{
	void* ret = NULL;

	if (!(closing && initialized))
	{
		cerr << LIB_ERROR_MESSAGE << endl;
		return WAIT_FOR_CLOSE_TO_EARLY;
	}
	if(pthread_join(daemonThread,&ret))
	{
		cerr << SYS_ERROR_MESSAGE << endl;
		return FAIL;
	}
	closing = false;
	//cout << howManyWritten(); //debug
	return WAITFORCLOSE_SUCCESSFUL;
}

