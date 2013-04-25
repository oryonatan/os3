/*
 * wrappedFunctions.h
 *
 *  Created on: Apr 26, 2013
 *      Author: maria
 *
 *      This class is used to make any system calls that fail throw an appropriate exception.
 */

#ifndef WRAPPEDFUNCTIONS_H_
#define WRAPPEDFUNCTIONS_H_

#include "exceptions.h"
#include "pthread.h"

void pthreadMutexLock (pthread_mutex_t* mutex)
{
	if (pthread_mutex_lock (mutex))
	{
		throw SystemErrorException();
	}
}

void pthreadMutexUnlock (pthread_mutex_t* mutex)
{
	if (pthread_mutex_unlock (mutex))
	{
		throw SystemErrorException();
	}
}



#endif /* WRAPPEDFUNCTIONS_H_ */
