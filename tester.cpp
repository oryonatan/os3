/*
 * tester.cpp
 *
 *  Created on: Apr 26, 2013
 *      Author: maria
 */
#pragma GCC diagnostic ignored "-Wwrite-strings"
#include "outputdevice.h"
#include "TaskList.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

int main ()
{
//	TaskList tl;
	initdevice("bla.txt");
	char * buf = "abcdefg\n";
	for (int var = 0; var < 1000; ++var) {
		write2device(buf,strlen(buf));
	}
	closedevice();
//
	wait4close();
	cout<<"\nFIN\n";
}
