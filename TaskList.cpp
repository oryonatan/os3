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
#include "safelocks.h"

#define TID_NOT_FOUND_ERROR -2
#define FLUSH_SUCCESFUL 1

int TaskList::addTask(vector<char> data, int length)
{
	try
	{
		safeLock(&listMutex);
		int tid = this->getFreeID();
		if (tid == FAIL)
		{
			safeUnlock(&listMutex);
			return FAIL;
		}
		//create new task and push
		shared_ptr<Task> newTask(new Task(tid, data, length));
		tasks.push(newTask);
		ids[tid] = newTask;
		safeUnlock(&listMutex);
		return tid;
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(listMutex);
		return FAIL;
	}
}

TaskList::TaskList() :
		tasks(), ids(), history()
{
	//ctor can fail with exception
	safeMutexInit(&freeIdMutex, NULL);
	safeMutexInit(&listMutex, NULL);
}
location TaskList::findTid(int tid)
{
	try
	{
		safeLock(&listMutex);

		if (ids.find(tid) != ids.end())
		{
			safeUnlock(&listMutex);
			return RUNNING;
		}
		else if (history.find(tid) != history.end())
		{
			safeUnlock(&listMutex);
			return HISTORY;
		}
		safeUnlock(&listMutex);
		return NOT_FOUND;
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(listMutex);
		return NOT_FOUND;
	}
}

void TaskList::done(int tid)
{
	try
	{
		safeLock(&listMutex);
		history.insert(tid);
		curRun = NULL;
		safeUnlock(&listMutex);
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(listMutex);
	}
}

int TaskList::waitToEnd(int tid)
{
	try
	{
		safeLock(&listMutex);
		shared_ptr<Task> toWait = NULL;
		if (curRun != NULL && curRun->id == tid)
		{
			toWait = curRun;
		}
		if (ids.find(tid) != ids.end())
		{
			toWait = ids.find(tid)->second;
		}
		if (toWait != NULL)//if there is such task
		{
			safeWait(&(toWait->sig), &listMutex);
			safeUnlock(&listMutex);
			return FLUSH_SUCCESFUL;
		}
		else if (history.find(tid) != history.end())//maybe in history?
		{
			safeUnlock(&listMutex);
			return FLUSH_SUCCESFUL;
		}
		//if you got here , it is not running , not waiting and not in history
		//so it does'nt exists
		safeUnlock(&listMutex);
		return TID_NOT_FOUND_ERROR;
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(listMutex);
		return FAIL;
	}
}

void TaskList::deleteAllTasks()
{
	//used by the dtor
	history.clear();
	while (!tasks.empty())
	{
		tasks.pop();
	}

}
TaskList::~TaskList()
{
	//dtor can throw exceptions
	safeLock(&listMutex);
	deleteAllTasks();
	safeMutexDestroy(&freeIdMutex);
	safeUnlock(&listMutex);
	safeMutexDestroy(&listMutex);
}

int TaskList::idsLeft()
{
	try
	{
		safeLock(&listMutex);
		//create copy
		int size = ids.size();
		safeUnlock(&listMutex);
		return size;
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(listMutex);
		return FAIL;
	}

}

int TaskList::getFreeID()
{
	try
	{
		safeLock(&freeIdMutex);
		//find first that is unused , probably there are some faster ways to do this
		//as this way we it takes O(number of tasks) to get new id, however this way is far simpler
		for (int i = 0; i <= INT_MAX; ++i)
		{
			if (ids.find(i) == ids.end())
			{
				safeUnlock(&freeIdMutex);
				return i;
			}
		}
		safeUnlock(&freeIdMutex);
		return FAIL;
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(freeIdMutex);
		return FAIL;
	}
}

shared_ptr<Task> TaskList::popTask()
{
	try
	{
		safeLock(&listMutex);
		if (tasks.empty())
		{
			safeUnlock(&listMutex);
			return NULL;
		}
		shared_ptr<Task> front = tasks.front();
		curRun = front;
		//clean!
		ids.erase(front->id);
		tasks.pop();
		safeUnlock(&listMutex);
		return front;
	} catch (PthreadError &e)
	{
		UNLOCK_IGNORE(listMutex);
		return NULL;
	}
}

