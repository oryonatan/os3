////============================================================================
//// Name        : OS_EX3.cpp
//// Author      :
//// Version     :
//// Copyright   : Your copyright notice
//// Description : Hello World in C++, Ansi-style
////============================================================================
//
//#include <iostream>
//#include "outputdevice.h"
//#include <sstream>
//#include <fstream>
//#include <unistd.h>
//
//#define NO_FLUSH -1
//#define FLUSH_ALL -2
//#define RANDOM_OPS -3
//#define BUSY_WAIT_ON_WASITWRITTEN -4
//#define SLEEP_CYCLE -5
//#define JEDI_MONKEYS -6
//
//using namespace std;
//
//string toStringLine(int a);
//void write_numbers(int start, int end);
//int* write_numbers_and_get_ids(int start, int end);
//void write_numbers_and_flush_every(int start, int end, int when);
//void busy_wait(unsigned long time);
//int check_numbers_written(char* filename, int start, int end);
//void flush_tasks(int* task_ids, int length);
//void launch_job(int num, int start, int end, int flush_every);
//
//typedef struct job {
//	int start;
//	int end;
//	int flush_every;
//} job;
//
//pthread_t thread[6];
//job jobs[6];
//
//int do_test_A() {
//	int ret;
//
//	cout << "(1) Initializing device to \"file1.txt\"." << endl;
//	char filename[] = "file1.txt";
//	ret = initdevice(filename);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(2) done, writing the numbers 1 to 5000 to the file." << endl;
//	write_numbers(1, 5001);
//
//	cout << "(3) closing device." << endl;
//	closedevice();
//	wait4close();
//
//	cout << "(4) checking that all numbers have been written" << endl;
//	ret = check_numbers_written(filename, 1, 5001);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(5) Success, opening new file \"file2.txt\"." << endl;
//	char filename2[] = "file2.txt";
//	ret = initdevice(filename2);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(6) writing the numbers 1 to 200 to the file." << endl;
//	int* task_ids = write_numbers_and_get_ids(1, 201);
//
//	cout << "(7) flushing all task_ids." << endl;
//	flush_tasks(task_ids, 200);
//
//	cout << "(8) calling wasitwritten on all task_ids (all should return 0)."
//			<< endl;
//	int i;
//	for (i = 0; i < 200; i++) {
//		ret = wasItWritten(task_ids[i]);
//		if (ret != 0) {
//			cout << "ERROR wasItWritten(" << task_ids[i] << " returned " << ret
//					<< endl;
//			return -1;
//		}
//	}
//
//	delete[] task_ids;
//
//	cout << "(9) closing device." << endl;
//	closedevice();
//	wait4close();
//
//	cout << "(10) checking that all numbers have been written" << endl;
//	ret = check_numbers_written(filename2, 1, 201);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(11) Success, opening new file \"file3.txt\"." << endl;
//	char filename3[] = "file3.txt";
//	ret = initdevice(filename3);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(12) writing the numbers 1 to 50 to the file and calling" << endl
//			<< "     flush2device() after each write" << endl;
//	write_numbers_and_flush_every(1, 51, 1);
//
//	cout << "(13) writing the numbers 51 to 100 to the file and calling"
//			<< endl << "     flush2device() every 5th write" << endl;
//	write_numbers_and_flush_every(51, 101, 5);
//
//	cout << "(14) writing the numbers 101 to 200 to the file and calling"
//			<< endl << "     flush2device() every 50th writes" << endl;
//	write_numbers_and_flush_every(101, 201, 50);
//
//	cout << "(15) closing device." << endl;
//	closedevice();
//	wait4close();
//
//	cout << "(16) checking that all numbers have been written" << endl;
//	ret = check_numbers_written(filename3, 1, 201);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(17) Initializing device to \"file1.txt\" again." << endl;
//	ret = initdevice(filename);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(18) done, writing the numbers 5001 to 6000 to the file." << endl;
//	write_numbers(5001, 6001);
//
//	cout << "(19) closing device." << endl;
//	closedevice();
//	wait4close();
//
//	cout << "(20) checking that all numbers have been written" << endl
//			<< "     including those written before." << endl;
//	ret = check_numbers_written(filename, 1, 6001);
//	if (ret != 0) {
//		return -1;
//	}
//
//	return 0;
//}
//
//int do_test_B() {
//	int ret;
//
//	cout << "(21) Initializing device to \"file4.txt\"." << endl;
//	char filename[] = "file4.txt";
//	ret = initdevice(filename);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(22) creating new thread to write numbers 1-200 while this thread"
//			<< endl << "     writes numbers 201-400." << endl;
//	launch_job(0, 1, 201, NO_FLUSH);
//	write_numbers(201, 401);
//
//	cout << "(23) all numbers submitted, joinning secondary thread." << endl;
//	pthread_join(thread[0], NULL);
//
//	cout << "(24) closing device." << endl;
//	closedevice();
//	wait4close();
//
//	cout << "(25) checking that all numbers from both threads"
//			<< " have been written" << endl;
//	ret = check_numbers_written(filename, 1, 401);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(26) success, initializing device to \"file5.txt\"." << endl;
//	char filename2[] = "file5.txt";
//	ret = initdevice(filename2);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(27) creating 3 new threads to write numbers 201-400 " << endl
//			<< "     401-600 601-800 while this thread writes numbers " << endl
//			<< "     1-200." << endl;
//	launch_job(0, 201, 401, NO_FLUSH);
//	launch_job(1, 401, 601, NO_FLUSH);
//	launch_job(2, 601, 801, NO_FLUSH);
//	write_numbers(1, 201);
//
//	cout << "(28) joinning secondary threads." << endl;
//	pthread_join(thread[0], NULL);
//	pthread_join(thread[1], NULL);
//	pthread_join(thread[2], NULL);
//
//	cout << "(29) closing device." << endl;
//	closedevice();
//	wait4close();
//
//	cout << "(30) checking that all numbers from all threads"
//			<< " have been written" << endl;
//	ret = check_numbers_written(filename2, 1, 801);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(31) success, initializing device to \"file6.txt\"." << endl;
//	char filename3[] = "file6.txt";
//	ret = initdevice(filename3);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(32) creating 3 threads to write numbers " << endl
//			<< "     2001-4000 with flush after each write" << endl
//			<< "     4001-6000 with flush after each 5 writes" << endl
//			<< "     6001-8000 with flush after each 50 writes" << endl
//			<< "     1-2000 by this thread with no flush." << endl;
//	launch_job(0, 2001, 4001, 1);
//	launch_job(1, 4001, 6001, 5);
//	launch_job(2, 6001, 8001, 50);
//	write_numbers(1, 2001);
//
//	cout << "(33) joinning secondary threads." << endl;
//	pthread_join(thread[0], NULL);
//	pthread_join(thread[1], NULL);
//	pthread_join(thread[2], NULL);
//
//	cout << "(34) closing device." << endl;
//	closedevice();
//	wait4close();
//
//	cout << "(35) checking that all numbers from all threads"
//			<< " have been written" << endl;
//	ret = check_numbers_written(filename3, 1, 8001);
//	if (ret != 0) {
//		return -1;
//	}
//
//	return 0;
//}
//
//int do_test_C() {
//	int ret;
//
//	cout << "(36) success, initializing device to \"file7.txt\"." << endl;
//	char filename[] = "file7.txt";
//	ret = initdevice(filename);
//	if (ret != 0) {
//		return -1;
//	}
//
//	cout << "(37) creating 6 threads to write numbers " << endl
//			<< "      201- 400 with flush after each write" << endl
//			<< "      401- 600 with long random operations between writes"
//			<< endl << "      601- 800 with busy wait on wasItWritten" << endl
//			<< "      801-1000 with flush in the end" << endl
//			<< "     1001-1200 with Jedi Monkeys fighting with banana lightsabers"
//			<< endl << "     1201-1400 with sleeping between writes" << endl
//			<< "        1- 200 by this thread with no flush." << endl;
//	launch_job(0, 201, 401, 1);
//	launch_job(1, 401, 601, RANDOM_OPS);
//	launch_job(2, 601, 801, BUSY_WAIT_ON_WASITWRITTEN);
//	launch_job(3, 801, 1001, FLUSH_ALL);
//	launch_job(4, 1001, 1201, JEDI_MONKEYS);
//	launch_job(5, 1201, 1401, SLEEP_CYCLE);
//	write_numbers(1, 201);
//
//	cout << "(38) joinning secondary threads." << endl;
//	cout << "     joinning flusher." << endl;
//	pthread_join(thread[0], NULL);
//	cout << "     joinning random ops." << endl;
//	pthread_join(thread[1], NULL);
//	cout << "     joinning wasItWritten() waiter." << endl;
//	pthread_join(thread[2], NULL);
//	cout << "     joinning end flusher." << endl;
//	pthread_join(thread[3], NULL);
//	cout << "     joinning the Jedi Monkyes." << endl;
//	pthread_join(thread[4], NULL);
//	cout << "     joinning sleeper." << endl;
//	pthread_join(thread[5], NULL);
//
//	cout << "(39) closing device." << endl;
//	closedevice();
//	wait4close();
//
//	cout << "(40) checking that all numbers from all threads"
//			<< " have been written" << endl;
//	ret = check_numbers_written(filename, 1, 1401);
//	if (ret != 0) {
//		return -1;
//	}
//
//	return 0;
//}
//
//int main() {
//
//	int ret = do_test_A();
//
//	if (ret != 0) {
//		cout << "ERROR, test failed" << endl;
//		return 0;
//	}
//
//	ret = do_test_B();
//
//	if (ret != 0) {
//		cout << "ERROR, test failed" << endl;
//		return 0;
//	}
//
//	ret = do_test_C();
//
//	if (ret != 0) {
//		cout << "ERROR, test failed" << endl;
//		return 0;
//	}
//
//	cout << endl << "Success, all tests completed!" << endl;
//
//	return 0;
//}
//
//string toStringLine(int a) {
//	ostringstream oss;
//	oss << a << endl;
//	return oss.str();
//}
//
//void write_numbers(int start, int end) {
//	string str;
//	int i;
//	for (i = start; i < end; i++) {
//		str = toStringLine(i);
//		write2device(const_cast<char*> (str.data()), str.length());
//	}
//}
//
//void write_numbers_and_flush_every(int start, int end, int when) {
//	string str;
//	int i, task_id;
//	for (i = start; i < end; i++) {
//		str = toStringLine(i);
//		task_id = write2device(const_cast<char*> (str.data()), str.length());
//		if (i % when == 0) {
//			flush2device(task_id);
//		}
//	}
//}
//
//int* write_numbers_and_get_ids(int start, int end) {
//	string str;
//	int i;
//	int* task_ids = new int[end - start];
//	for (i = start; i < end; i++) {
//		str = toStringLine(i);
//		task_ids[i - start] = write2device(const_cast<char*> (str.data()),
//				str.length());
//	}
//	return task_ids;
//}
//
//void busy_wait(unsigned long time) {
//	unsigned int t;
//	int a = 5, b = 0, c = 100;
//	for (t = 0; t < time; t++) {
//		a = b + c;
//		c = b / 2;
//		a++;
//		c += 1000;
//	}
//}
//
//int check_numbers_written(char* filename, int start, int end) {
//	ifstream file(filename, ifstream::in);
//	if (!file.is_open()) {
//		cerr << "ERROR, could not open file " << filename << " for checking"
//				<< endl;
//		return -1;
//	}
//	int length = end - start;
//	bool* checked = new bool[length];
//	int i;
//	for (i = 0; i < length; i++) {
//		checked[i] = false;
//	}
//
//	int n;
//	file >> n;
//	while (file.good()) {
//		i = n - start;
//		if (checked[i]) {
//			cerr << "ERROR, in file " << filename << " the number " << n
//					<< " was written twice" << endl;
//			return -1;
//		}
//		checked[i] = true;
//		file >> n;
//	}
//
//	for (n = start; n < end; n++) {
//		if (!checked[n - start]) {
//			cerr << "ERROR, in file " << filename << " the number " << n
//					<< " was not written" << endl;
//			return -1;
//		}
//	}
//
//	file.close();
//
//	delete[] checked;
//
//	return 0;
//}
//
//void flush_tasks(int* task_ids, int length) {
//	int i;
//	for (i = 0; i < length; i++) {
//		flush2device(task_ids[i]);
//	}
//}
//
//void write_numbers_and_random_ops(int start, int end) {
//	string str;
//	int i;
//	for (i = start; i < end; i++) {
//		str = toStringLine(i);
//		write2device(const_cast<char*> (str.data()), str.length());
//		busy_wait(i % 50000);
//	}
//}
//
//void write_numbers_and_sleep(int start, int end) {
//	string str;
//	int i;
//	for (i = start; i < end; i++) {
//		str = toStringLine(i);
//		write2device(const_cast<char*> (str.data()), str.length());
//		if (i % 4 == 0) {
//			sleep(i % 2);
//		}
//		if (i % 40 == 0) {
//			cout << "         sleeper thread completed " << i - start + 1
//					<< " out of " << end - start << endl;
//		}
//	}
//}
//
//void write_numbers_and_nag_wasitwritten(int start, int end) {
//	string str;
//	int i, task_id;
//	for (i = start; i < end; i++) {
//		str = toStringLine(i);
//		task_id = write2device(const_cast<char*> (str.data()), str.length());
//		while (wasItWritten(task_id) != 0) {
//			busy_wait(5000);
//		}
//	}
//}
//
//void* do_job(void* jobP) {
//	job* j = (job*) jobP;
//	int start = j->start;
//	int end = j->end;
//
//	if (j->flush_every == FLUSH_ALL) {
//		int* ids = write_numbers_and_get_ids(start, end);
//		flush_tasks(ids, end - start);
//		delete[] ids;
//	} else if (j->flush_every > 0) {
//		write_numbers_and_flush_every(start, end, j->flush_every);
//	} else if (j->flush_every == RANDOM_OPS) {
//		write_numbers_and_random_ops(start, end);
//	} else if (j->flush_every == BUSY_WAIT_ON_WASITWRITTEN) {
//		write_numbers_and_nag_wasitwritten(start, end);
//	} else if (j->flush_every == SLEEP_CYCLE) {
//		write_numbers_and_sleep(start, end);
//	} else {
//		write_numbers(start, end);
//	}
//
//	return NULL;
//}
//
//void launch_job(int num, int start, int end, int flush_every) {
//	jobs[num].start = start;
//	jobs[num].end = end;
//	jobs[num].flush_every = flush_every;
//	pthread_create(&thread[num], NULL, do_job, &jobs[num]);
//}
