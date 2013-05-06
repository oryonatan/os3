/*
 * TaskList.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: maria
 */

#include "TaskList.h"
#include <limits.h>
#include <memory>
#include "unistd.h"

#define TID_NOT_FOUND_ERROR -2
#define FLUSH_SUCCESFUL 1

int TaskList::addTask(vector<char> data,int length)
{
	pthread_mutex_lock(&listMutex);
	int tid = this->getFreeID();
	if (tid == FAIL)
	{
		pthread_mutex_unlock(&listMutex);
		return FAIL;
	}
	shared_ptr<Task> newTask(new Task(tid, data,length));
	tasks.push(newTask);
	ids[tid] = newTask;
	pthread_mutex_unlock(&listMutex);
	return tid;
}

TaskList::TaskList() :
		tasks(), ids(), history()
{
	pthread_mutex_init(&freeIdMutex, NULL);
	pthread_mutex_init(&listMutex, NULL);
}
location TaskList::findTid(int tid)
{
	pthread_mutex_lock(&listMutex);

	if (ids.find(tid) != ids.end())
	{
		pthread_mutex_unlock(&listMutex);
		return RUNNING;
	}
	else if (history.find(tid) != history.end())
	{
		pthread_mutex_unlock(&listMutex);
		return HISTORY;
	}
	pthread_mutex_unlock(&listMutex);
	return NOT_FOUND;
}


void TaskList::done(int tid)
{
	pthread_mutex_lock(&listMutex);
	history.insert(tid);
	curRun=NULL;
	pthread_mutex_unlock(&listMutex);
}

int TaskList::waitToEnd(int tid)
{
	pthread_mutex_lock(&listMutex);
	shared_ptr<Task> toWait = NULL;
	if (curRun!=NULL&& curRun->id == tid)
	{
		toWait = curRun;
	}
	if (ids.find(tid) != ids.end())
	{
		toWait=ids.find(tid)->second;
	}
	if (toWait != NULL)
	{
		pthread_cond_wait(&(toWait->sig),&listMutex);
		pthread_mutex_unlock(&listMutex);
		return FLUSH_SUCCESFUL;
	}
	else if (history.find(tid) != history.end())
	{
		pthread_mutex_unlock(&listMutex);
		return FLUSH_SUCCESFUL;
	}
	pthread_mutex_unlock(&listMutex);
	return TID_NOT_FOUND_ERROR;
}

void TaskList::deleteAllTasks()
{

	history.clear();
	while (!tasks.empty())
	{
		tasks.pop();
	}

}
//TODO - why do we need it like this?
TaskList::~TaskList()
{
	pthread_mutex_lock(&listMutex);
	deleteAllTasks();
	pthread_mutex_destroy(&freeIdMutex);
	pthread_mutex_unlock(&listMutex);
	pthread_mutex_destroy(&listMutex);
}


int TaskList::idsLeft()
{
	pthread_mutex_lock(&listMutex);
	int size=ids.size();
	pthread_mutex_unlock(&listMutex);
	return size;

}

int TaskList::getFreeID()
{
	pthread_mutex_lock(&freeIdMutex);
	for (int i = 0; i <= INT_MAX; ++i)
	{
		if (ids.find(i) == ids.end())
		{
			pthread_mutex_unlock(&freeIdMutex);
			return i;
		}
	}
	pthread_mutex_unlock(&freeIdMutex);
	return FAIL;
}


shared_ptr<Task> TaskList::popTask()
{
	pthread_mutex_lock(&listMutex);
	if (tasks.empty())
	{
		pthread_mutex_unlock(&listMutex);
		return NULL;
	}

	shared_ptr<Task>front = tasks.front();
	curRun=front;
	ids.erase(front->id);
	tasks.pop();
	pthread_mutex_unlock(&listMutex);
	return front;
}

