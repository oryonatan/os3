#include "safelocks.h"
#include "errno.h"
void safeMutexInit(pthread_mutex_t* __mutex,
		const pthread_mutexattr_t* __mutexattr)
{
	pthread_mutex_init(__mutex,__mutexattr);
}

void safeCondInit(pthread_cond_t* __cond, const pthread_condattr_t* __condattr)
{
	if (pthread_cond_init(__cond, __condattr))
	{
		throw PthreadError();
	}
}

void safeLock(pthread_mutex_t* __mutex)
{
	int ret = pthread_mutex_lock(__mutex);
	if (ret )
	{
		throw PthreadError();
	}
}

void safeUnlock(pthread_mutex_t* __mutex)
{
	if (pthread_mutex_unlock(__mutex))
	{
		throw PthreadError();
	}
}

void safeBroadCast(pthread_cond_t *__cond)
{
	if (pthread_cond_broadcast(__cond))
	{
		throw PthreadError();
	}
}

void safeCondDestroy(pthread_cond_t* __cond)
{
	if (pthread_cond_destroy(__cond))
	{
		throw PthreadError();
	}
}

void safeMutexDestroy(pthread_mutex_t* __mutex)
{
	int ret = pthread_mutex_destroy(__mutex);
	if (ret==EINVAL) // if it was destroyed before in an ugly way for some reason
	{
		*__mutex = PTHREAD_MUTEX_INITIALIZER;
		return;
	}
	else if(ret)
	{
		throw PthreadError();
	}
	//set to static initializer for future use.
	*__mutex = PTHREAD_MUTEX_INITIALIZER;
}

void safeWait(pthread_cond_t *__restrict __cond,
		pthread_mutex_t *__restrict __mutex)
{
	if (pthread_cond_wait(__cond, __mutex))
	{
		throw PthreadError();
	}
}

void safeJoin(pthread_t __th, void** __thread_return)
{
	if (pthread_join(__th, __thread_return))
	{
		throw PthreadError();
	}
}

