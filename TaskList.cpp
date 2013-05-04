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

int TaskList::addTask(shared_ptr<string> data)
{
	pthread_mutex_lock(&listMutex);
	int tid = this->getFreeID();
	if (tid == FAIL)
	{
		pthread_mutex_unlock(&listMutex);
		return FAIL;
	}
	shared_ptr<Task> newTask(new Task(tid, data));
	tasks.push(newTask);
	ids[tid] = newTask;
	pthread_mutex_unlock(&listMutex);
	return tid;
}

TaskList::TaskList() :
		tasks(), ids(), history()
{
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

pthread_mutex_t* TaskList::getSignalMutex(int tid) const
{
	return &(ids.find(tid)->second->mut);
}

pthread_cond_t* TaskList::getSignal(int tid) const
{
	return &(ids.find(tid)->second->sig);
}

void TaskList::done(int tid)
{
	history.insert(tid);
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
	deleteAllTasks();
	pthread_mutex_destroy(&freeIdMutex);
	pthread_mutex_destroy(&listMutex);
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

shared_ptr<Task> TaskList::front() const
{
	if (tasks.empty())
		return NULL;
	return tasks.front();
}

int TaskList::popTask()
{
	pthread_mutex_lock(&listMutex);
	ids.erase(front()->id);
	tasks.pop();
	pthread_mutex_unlock(&listMutex);
	return OKAY;
}
