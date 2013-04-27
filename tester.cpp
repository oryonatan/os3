/*
 * tester.cpp
 *
 *  Created on: Apr 26, 2013
 *      Author: maria
 */

#include "outputdevice.h"
#include "TaskList.h"
#include <stdio.h>
#include <string.h>

int main ()
{
	TaskList * tl = new TaskList();

	char * buf = "sadsadsa";
	char * data = (char*)malloc(strlen(buf));
	tl->addTask(data);
	Task* ts = tl->popTask();
	delete ts;
	data = (char*)malloc(strlen(buf));
	tl->addTask(data);
	delete ts;
	cout<<"\nFIN\n";
}




