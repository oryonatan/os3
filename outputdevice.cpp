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

#define LIB_ERROR_MESSAGE "Output device library error"
#define SYS_ERROR_MESSAGE "system error"

#define YES 0
#define NO 1

//constant return values as determined in Ex3
#define SUCCESS 0
#define FILESYSTEM_ERROR -2
#define TID_NOT_FOUND_ERROR -2
#define WAITFORCLOSE_SUCCESSFUL 1
#define FAIL -1

#define APPEND "a"

using namespace std;

//The mutex that makes sure only one initialization process goes at the same time
pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;
// Conditional variable that signifies that printing queue is empty
pthread_cond_t empty;
//The mutex associated with the conditional variable
pthread_mutex_t emptyMut;
//The daemon thread that does all the printing
pthread_t daemonThread;

//Signifies if the device is currently closing
bool closing = false;
//Signifies whether an initializing function was called succesfully
bool initialized = false;
unique_ptr<TaskList> allTasks(new TaskList);
FILE* diskFile = NULL;
int printCounter = 0;



//Helper function that destroys all the resources and frees the memory
void closeEverything()
{

	if (diskFile != NULL)
	{
		//TODO - Yonatan - shouldn't we check if fclose succeeds?
		fclose(diskFile);
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
		firstTask = allTasks->front();
		if (firstTask != NULL)
		{
			//cerr<<"wrote something\n"; //debug
			fprintf(diskFile, firstTask->data->c_str(), "%s");
			printCounter++;
			pthread_cond_broadcast(&(firstTask->sig));
			allTasks->popTask();
		}
		else
		{
			pthread_cond_broadcast(&empty);
			// if the thread runs after closeDevice was called (
			if (closing)
			{
				closeEverything();
			}
		}
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
	if (initialized || filename == NULL)
	{
		pthread_mutex_unlock(&initMutex);
		cerr << LIB_ERROR_MESSAGE << endl;
		return FAIL;
	}

	if ((diskFile = fopen(filename, APPEND)) == NULL)
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
	shared_ptr<string> data(new string(buffer)) ;
	newId = allTasks->addTask(data);
	return newId;
}

int flush2device(int task_id)
{
	if (!initialized || task_id < 0)
	{
		cerr << LIB_ERROR_MESSAGE << endl;
		return FAIL;
	}
	location loc = allTasks->findTid(task_id);
	switch (loc)
	{
	case NOT_FOUND:
		cerr << LIB_ERROR_MESSAGE << endl;
		return FAIL;
	case HISTORY:
		return OKAY;
	case RUNNING:
		pthread_mutex_lock(allTasks->getSignalMutex(task_id));
		pthread_cond_wait(allTasks->getSignal(task_id),
				allTasks->getSignalMutex(task_id));
		return OKAY;

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
	//TODO - Yonatan - aren't we supposed to call closeDevice somewhere?
	//why would closing be true?
	if (!closing)
	{
		cerr << LIB_ERROR_MESSAGE << endl;
		return FAIL;
	}
	pthread_mutex_lock(&emptyMut);
	pthread_cond_wait(&empty, &emptyMut);
	pthread_mutex_destroy(&emptyMut);
	closeEverything();
	closing = false;
	pthread_join(daemonThread,&ret);
	//cout << howManyWritten(); //debug
	return WAITFORCLOSE_SUCCESSFUL;
}

