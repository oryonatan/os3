/*
 * outputdevice.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: maria
 */


int initdevice(char *filename);

int write2device(char *buffer, int length);

int flush2device(int task_id);

int wasItWritten(int task_id);

int howManyWritten();

void closedevice();

int wait4close();




