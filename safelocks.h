/*
 * safelocks.h
 *
 *  Created on: May 11, 2013
 *      Author: yonatan
 */

#ifndef SAFELOCKS_H_
#define SAFELOCKS_H_
#include "pthread.h"
#include <exception>
using namespace std;
#define UNLOCK_IGNORE(mutex) 	try{safeUnlock(&mutex);} catch (PthreadError &e){};

void safeMutexInit(pthread_mutex_t *__mutex,__const pthread_mutexattr_t *__mutexattr);
void safeCondInit(pthread_cond_t *__mutex, __const pthread_condattr_t *__mutexattr);
void safeLock(pthread_mutex_t *__mutex);
void safeUnlock(pthread_mutex_t *__mutex);
void safeBroadCast(pthread_cond_t *__cond);
void safeCondDestroy(pthread_cond_t *__cond);
void safeMutexDestroy(pthread_mutex_t *__mutex);
void safeWait(pthread_cond_t *__restrict __cond,pthread_mutex_t *__restrict __mutex);
void safeJoin(pthread_t __th, void **__thread_return);
class PthreadError:public exception{};


#endif /* SAFELOCKS_H_ */
