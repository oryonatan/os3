/*
 * outputdevice.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: maria
 */

#include <iostream>
#include "pthread.h"
#include <algorithm>

#include "outputdevice.h"
#include "TaskQueue.h"
#include "printTask.h"
#include "exceptions.h"
#include "IDList.h"
#include "wrappedFunctions.h"

#define SUCCESS 0
#define FILESYSTEM_ERROR -2
#define TID_NOT_FOUND_ERROR -2
#define WAITFORCLOSE_SUCCESSFUL 1
#define FAIL -1
#define APPEND "a"

using namespace std;

pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writeMutex;

bool initialized = false;
IDList* finishedIDs = NULL;
IDList* unfinishedIDs = NULL;
TaskQueue* printQueue = NULL;
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

	if (finishedIDs != NULL)
	{
		delete printQueue;
	}

	if (unfinishedIDs!= NULL)
	{
		delete printQueue;
	}

	if (diskFile != NULL )
	{
		fclose (diskFile);
	}

	initialized = false;
	printCounter = 0;

}


//The entry point to the writing daemon thread
void* writingFunc (void *)
{
	void* p;
	return p;
}


int initdevice(char *filename)
{
	try
	{

		pthreadMutexLock(&initMutex);
		if (initialized)
		{
			throw LibraryErrorException();
		}

		if (filename == NULL)
		{
			throw LibraryErrorException();
		}

		if ((diskFile = fopen (filename, APPEND)) == NULL)
		{
			throw FilesystemErrorException();
		}

		printQueue = new TaskQueue();
		finishedIDs = new IDList();
		unfinishedIDs = new IDList();

	//TODO create the daemon thread that would do all the writing
	//TODO unlock the unitializing mutex and destroy it
		pthreadMutexUnlock(&initMutex);
	}
	catch (SystemErrorException& e)
	{
		closeEverything();
		pthread_mutex_unlock(&initMutex);
		return FAIL;
	}

	catch (FilesystemErrorException& e)
	{
		closeEverything();
		pthread_mutex_unlock(&initMutex);
		return FILESYSTEM_ERROR;
	}
	initialized = true;
	return SUCCESS;
}

int write2device(char *buffer, int length)
{
	try{
		if (!initialized)
		{
			throw LibraryErrorException();
		}
		int newID;
		//if this is the first task we create, the ID is 0
		if (finishedIDs->isEmpty() && unfinishedIDs->isEmpty())
		{
			newID = 0;
		}
		//else, it is either the lowest free ID or the next ID
		else
		{
			newID = min (finishedIDs->lowestID(), unfinishedIDs->highestID() +1);
		}

		printTask newTask(newID, buffer, length);
		//TODO create a new writing task
		//TODO push it to the task queue
		//TODO push its id to unfinished list
	}
	catch (LibraryErrorException& e)
	{

	}

	return SUCCESS;
}

int flush2device(int task_id)
{
	return SUCCESS;
}

int wasItWritten(int task_id)
{
	//TODO check in unfinishedIDS
	//TODO if not found, check in finishedIDs
	return SUCCESS;
}

int howManyWritten()
{
	return 0;
}

void closedevice()
{
}

int wait4close()
{
	return WAITFORCLOSE_SUCCESSFUL;
}




