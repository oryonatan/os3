/*
 * TaskList.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: maria
 */

#include "TaskList.h"
#include <limits.h>
#include <memory>

int TaskList::addTask(char* data)
{
	int tid = this->getFreeID();
	if (tid == FAIL)
	{
		return FAIL;
	}
	tasks.push(new Task(tid, data));
	return tid;
}

TaskList::TaskList() :
		tasks(), ids(), history()
{
}

location TaskList::findTid(int tid) const
{
	if(ids.find(tid) != ids.end() )
	{
		return RUNNING;
	}
	else if(history.find(tid) != history.end()){
		return HISTORY;
	}
	return NOT_FOUND;
}

pthread_mutex_t* TaskList::getSignalMutex(int tid) const
{
	return &(ids.find(tid)->second->mut);
}

pthread_cond_t*  TaskList::getSignal(int tid) const
{
	return &(ids.find(tid)->second->sig);
}

void TaskList::delteAllTasks()
{
	while (!tasks.empty())
	{
		delete (tasks.front());
		tasks.pop();
	}
}

TaskList::~TaskList()
{
	delteAllTasks();
}

int TaskList::getFreeID()
{
	for (int i = 0; i <= INT_MAX; ++i)
	{
		if (ids.find(i) == ids.end())
		{
			return i;
		}
	}
	return FAIL;
}

Task * TaskList::popTask()
{
	//TODO: check that dtor is not called before here
	Task * ret = tasks.front();
	tasks.pop();
	ids.erase(ret->id);
	return ret;
}
