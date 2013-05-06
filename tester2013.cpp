/*
 * Tester for Ex3.
 * Written by Alon Ben-Shimol
 */
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <climits>
#include <cmath>
#include <string>
#include <cstring>
#include <assert.h>
#include <unistd.h>
#include "outputdevice.h"
using namespace std;

#define NUM_RANGE 100
#define FLUSH_RANGE 250
#define NUM_OF_THREADS 50
#define BUF_PER_THREAD 30
#define TEST_12_FLUSH 50

#define FAILED_TEST(x,y) { printf("Failed test %s. \n\tReason: %s.\n", x, y); }
#define PASSED_TEST(x) { printf("Passed test %s.\n", x); }


static size_t getSizeOfFile( FILE* );
static int compareFileToString ( const char* , const char* );
static int findStringInFile ( FILE* , const char* );
static void generate_word( char* , size_t , int );
static size_t countDigits( size_t );
static int fileNotSorted (const char * const );

int test_13_max;

static int compareFileToString ( const char* f_path, const char* str )
{
	FILE *f = fopen(f_path, "r");
	char* file_Content;
	size_t fileSize = 0;

	fileSize = getSizeOfFile(f);

	// + 1 for '\0'
	file_Content = (char*)malloc(fileSize + 1);

	//Read the file
	fread(file_Content,fileSize, sizeof(char) ,f);
	file_Content[fileSize]='\0'; // Add NULL char.

	if ( strcmp (file_Content, str) != 0 )
	{
		cout << "got "<< file_Content <<"instead of " << str <<endl;
		return 1;
	}

	free(file_Content);
	fclose(f);
	return 0;
}

static size_t getSizeOfFile (FILE* f)
{
	fseek(f, 0L, SEEK_END); // Determine size of file
	size_t fileSize = ftell(f);
	fseek(f, 0L, SEEK_SET);

	return fileSize;
}

static int findStringInFile ( FILE* f, const char* str )
{
	if ( !f )
	{
		printf("\nUnable to open %s for testing. \nTest stopped!", str);
		return 1;
	}

	char tmp[LINE_MAX]={0};
	while( fgets(tmp, sizeof(tmp), f) !=NULL )
	{
		if ( strcmp(tmp, str) == 0 ) // We found the string we we're looking for
		{
			return 0;
		}
	}

	return 1;
}



static int countStringInFile ( FILE* f, const char* str )
{
	size_t count = 0;

	if ( !f )
	{
		printf("\nUnable to open %s for testing. \nTest stopped!", str);
		return 1;
	}

	char tmp[LINE_MAX]={0};
	while( fgets(tmp, sizeof(tmp), f) != NULL )
	{
		if ( strcmp(tmp, str) == 0 ) // We found the string we we're looking for
		{
			count++;
		}
	}

	return count;
}


static void generate_word(char* str, size_t len, int longW)
{
	size_t i;
	for (i = 0; i < len - 2 ; ++i)
	{
		if ( longW && ( i % 186 == 0 ) )
		{
			str[i] = '\n';
			continue;
		}
		str[i] = rand() % (126 - 33 + 1) + 33;
	}
	str[len - 2] = '\n';
	str[len - 1] = '\0';
		//str[len] = '\0';
}


static size_t countDigits( size_t number )
{
	int count = 0;
	do
	{
		count++;
		number = number / 10;

	} while (number != 0);

	return count;
}


static int fileNotSorted (const char * const fName )
{
	FILE *fp= fopen(fName,"r");
	if ( !fp )
	{
		printf("Unable to open %s for testing. Test stopped!", fName);
		return 1;
	}

	char temp_After [LINE_MAX/100]={0};
	fgets(temp_After, sizeof(temp_After), fp);

	char* temp_Before = (char*)malloc( sizeof(char) * (strlen(temp_After) + 1) );
	strncpy(temp_Before, temp_After, strlen(temp_After) + 1);

	while( fgets(temp_After, sizeof(temp_After), fp) !=NULL )
	{
		if ( atoi(temp_After) != atoi(temp_Before) + 1 )
		{
			free (temp_Before);
			return atoi(temp_After);
		}

		free(temp_Before);

		temp_Before = (char*)malloc( sizeof(char) * (strlen(temp_After) + 1) );
		strncpy(temp_Before, temp_After, strlen(temp_After) + 1);
	}

	free (temp_Before);

	fclose(fp);
	return -1;
}


// Init and close device
void test_1()
{
	if ( initdevice("./Tests/test_1") != 0 )
	{
		FAILED_TEST("1", "Unable to initialize device correctly");
		exit(-3);
	}

	closedevice();
	wait4close();
	PASSED_TEST("1"); //Manage to close device
}


//Multiply initializations (not-allowed) verify
//Multiply closures (allowed) verify
void test_2()
{
	initdevice("./Tests/test_2");
	if ( !initdevice("./Tests/test_2") )
	{
		FAILED_TEST("2", "Allowing multiply inits before a device is closed");
		exit(-3);
	}

	closedevice();
	closedevice();
	closedevice();
	closedevice();
	wait4close();
	PASSED_TEST("2");
}

void test_3()
{
	closedevice();
	wait4close();
	PASSED_TEST("3");
}

// verify that init is prerequisite for all other functions
// and that closedevice() is for wait4close()
// Note: Error messages should be printed here - but are disabled
void test_4()
{
	srand (time(NULL));

	if ( write2device("B", 1) != -1 || flush2device(1) == 1 )
	{
		FAILED_TEST("4", "write2device() or flush2device() assumes init was called");
		exit(-3);
	}

	int id = rand(), ret_val = wasItWritten(id);

	if ( ret_val == 0 || ret_val == 1 )
	{
		FAILED_TEST("4", "wasitwritten() assumes init was called.");
		exit(-3);
	}

	if ( howManyWritten() != -1 )
	{
		FAILED_TEST("4", "howManyWritten() assumes init was called");
		exit(-3);
	}

	if ( wait4close() != -2 )
	{
		FAILED_TEST("4", "wait4close() assumes closeDevice() was called");
		exit(-3);
	}

	PASSED_TEST("4");
}


//Legal id's check
void test_5()
{
	srand (time(NULL));
	int id = rand();
	initdevice("./Tests/test_3");

	if ( flush2device(-1) != -2 ||
		 flush2device(0)  != -2 ||
		 flush2device(1)  != -2 ||
		 flush2device(id) != -2 )
	{
		cout << flush2device(-1) <<  flush2device(0)<<  flush2device(1)<<  flush2device(id) ;
		FAILED_TEST("5", "No tasks were given yet. All id's should fail to flush");
		exit(-3);
	}

	srand (time(NULL));
	id = rand();

	if ( wasItWritten(-1) != -2 ||
		 wasItWritten(0)  != -2 ||
		 wasItWritten(1)  != -2 ||
		 wasItWritten(id) != -2 )
	{

			FAILED_TEST("5", "No tasks were given yet. All id's should fail to flush");
			exit(-3);
	}

//	if ( howManyWritten() != 0 )
//	{
//		FAILED_TEST("5", "Number of tasks written should be 0 at this point" );
//		exit(-3);
//	}

	closedevice();
	wait4close();
	PASSED_TEST("5");
}

//simple buffer write - static allocation.
//Note: CHECK VALGRIND FOR THIS TEST!
//NOTE 2: If you get "still reachable memory" - this is NOT a leak
void test_6( char* buf, size_t len , int a )
{
	remove ("./Tests/test_6_7");
	initdevice("./Tests/test_6_7");
	int ret_val = write2device(buf, len);

	if (ret_val != 0)
	{
		if (a) { FAILED_TEST("6", "Writing task should have got the id 0 at this point"); exit(-3); }
		else {FAILED_TEST("7", "Writing task should have got the id 0 at this point"); exit(-3); }

	}
	closedevice();
	wait4close();

	if ( compareFileToString("./Tests/test_6_7", "test_6_7 is a very simple test" ) )
	{
		if (a) { FAILED_TEST("6","Wront match requested string and on file string"); exit(-3);}
		else {	FAILED_TEST("7", "Wront match requested string and on file string"); exit(-3);}

	}

	if ( a ) { PASSED_TEST("6"); }
	else {	PASSED_TEST("7"); }

	fflush(stdout);
}


//simple buffer write - dynamic allocation.
//Note: CHECK VALGRIND FOR THIS TEST!
void test_7()
{
	char* a = "test_6_7 is a very simple test";
	char* buf = (char*)malloc(strlen(a)+1);
	strncpy(buf, a, strlen(a)+1);
	test_6(buf, strlen(buf), 0);
	free (buf);
}


// simple buffer write. Verify append to file
void test_8()
{
	FILE* toCheck = fopen("./Tests/test_8", "w+");
	fwrite("abc", sizeof(char), strlen("abc"), toCheck);
	fflush(toCheck);
	fclose(toCheck);

	initdevice("./Tests/test_8");

	if ( write2device("defg", strlen("defg") ) == -1 )
	{
		FAILED_TEST("8", "Unable to write a buffer to file."); exit(-3);
	}

	closedevice();
	wait4close();

	if ( compareFileToString("./Tests/test_8", "abcdefg" ) ) {
		FAILED_TEST("8", "You didn't open the file with 'append' flag");
		exit(-3);
	}

	PASSED_TEST("8");
}

//Writing a very large buffer to file
void test_9()
{
	remove("./Tests/test_9");
	initdevice("./Tests/test_9");

	// Don't change this factor - or you get very large file in your system
	char* buf = (char*)malloc(INT_MAX/5000 + 1);
	generate_word (buf, INT_MAX/5000 , 1);

	int ret_1, ret_2;

	ret_1 = write2device(buf, strlen(buf));
	if (ret_1 == -1)
	{
		FAILED_TEST("9", "Unable to write buffer to device.");
		return;
	}
	free(buf);
	usleep(100000); // give random enough time

	char* buf2 = (char*)malloc(INT_MAX/5000 + 1);

	generate_word (buf2, INT_MAX/5000 , 1);

	ret_2 = write2device(buf2, strlen(buf2));
	if (ret_2 == -1 )
	{
		FAILED_TEST("9", "Unable to write buffer to device.");
		exit(-3);
	}


flush2device(ret_1); // if already written - we don't care
flush2device(ret_2);


	if ( howManyWritten() != 2 )
	{

		FAILED_TEST("9", "You should have write 2 buffers to the file at this point"); exit(-3);
	}


	closedevice();
	wait4close();

	FILE* f = fopen("./Tests/test_9", "r");

	int file_Size = getSizeOfFile(f), expected_Size = 2 * (INT_MAX/5000 ) - 2;
	if ( file_Size != expected_Size )
	{
		FAILED_TEST("9", "Mismatch between ammount of chars in file and the expected value");
		printf("\tExpected value is %d, and you wrote %d chars to the file.\n", expected_Size, file_Size);
		exit(-3);

	}
	fclose(f);

	PASSED_TEST("9");
}


// Advanced buffer writer - write numbers 1 - NUM_RANGE to file
// and check to indeed they all we're written to the file
// 'Possibly lost' valgrind 'failures' is because valgrind doesn't quite support
// pthread library. This is NOT a memory leakage.
void test_10()
{
	remove("./Tests/test_10");
	initdevice("./Tests/test_10");

	char* newNum;
	int i = 0;

	/* Note: if using thread based library for each task, then creating more than about 100
	 * threads may overflow the system. Take that in mind.
	 */
	for ( i = 0 ; i < NUM_RANGE; ++i)
	{
		newNum = (char*)malloc( countDigits(i) + 2 );
		sprintf(newNum,"%d\n", i);
		size_t len = strlen(newNum);
		if ( write2device( newNum, len ) == -1 )
		{
			FAILED_TEST("10", "Unable to write to file");
			exit(-3);
		}
		free(newNum);
	}

	closedevice();
	wait4close();

	// Check that numbers 0 - NUM_RANGE(-1) appear in the file
	FILE* f = fopen("./Tests/test_10", "r");
	for ( i = 0 ; i < NUM_RANGE; ++i)
	{
		newNum = (char*)malloc( countDigits(i) + 2 );
		sprintf(newNum,"%d\n", i);

		if ( findStringInFile(f, newNum) )
		{
			FAILED_TEST("10", "Unable to locate a number in the file './Tests/test_10'");
			printf("\tMissing number: %s ", newNum);
			exit(-3);
		}
		free(newNum);
		rewind(f);
	}

	fclose(f);


	PASSED_TEST("10");
}


void test_11()
{
	remove("./Tests/test_11");
	initdevice("./Tests/test_11");

	char* newNum;
	int i = 0;


	for ( i = 0 ; i < FLUSH_RANGE; ++i)
	{
		newNum = (char*)malloc( countDigits(i) + 2 );
		sprintf(newNum,"%d\n", i);
		size_t len = strlen(newNum);
		if ( flush2device(write2device( newNum, len ) ) == -1 )
		{
			FAILED_TEST("11", "Unable to write to file");
		}
		free(newNum);
	}

	closedevice();
	wait4close();

	// Check that numbers 0 - NUM_RANGE(-1) appear in the file
	FILE* f = fopen("./Tests/test_11", "r");
	for ( i = 0 ; i < FLUSH_RANGE; ++i)
	{
		newNum = (char*)malloc( countDigits(i) + 2 );
		sprintf(newNum,"%d\n", i);

		if ( findStringInFile(f, newNum) )
		{
			FAILED_TEST("11", "Unable to locate a number in the file './Tests/test_11'");
			printf("\tMissing number: %s ", newNum);
			exit(-3);
		}
		free(newNum);
		rewind(f);
	}

	fclose(f);

	// Now we check that they appear in ascending order since we flush after creating each task.
	int ret_val = fileNotSorted("./Tests/test_11");
	if (ret_val != -1)
	{
		FAILED_TEST("11", "File created in not sorted properly");
		printf("The number %d breaks the order in file test_11", ret_val);
		exit(-3);
	}

	PASSED_TEST("11");
}


// Check that you assign lower id numbers and reuse id numbers
// Also - checks howManyWritten (given flush) and that was it written indeed works.
// Note that according to forum clarification, wasItWritten should return true once
// a task has been written and until a new task with the same id will be created.
// Even if currently you don't have a task with id that was previously written, it should
// still return TRUE (i.e. 0)
void test_12()
{
     remove("./Tests/test_12");
     initdevice("./Tests/test_12");


     char* newBuf;
     int newBufLen;
     int id, maxId = 0;

     for ( int i = 0 ; i < TEST_12_FLUSH ; ++i )
     {
    	 newBuf = (char*)malloc( countDigits(i) + 2 );
    	 sprintf(newBuf,"%d\n", i);
    	 newBufLen = strlen(newBuf);

    	 id = write2device(newBuf, newBufLen);
    	 if (id >= TEST_12_FLUSH)
         {
             FAILED_TEST("12", "You assign higher id's than you should");
             printf("\tAt this stage, highest id should be %d\n", TEST_12_FLUSH);
             exit(-3);
         }
    	 if (id > maxId) maxId = id;
         free(newBuf);
     }

     for ( int i = 0 ; i < TEST_12_FLUSH ; ++i)
          flush2device(i); //Possible -2 made be returned here, but we dont care
     if ( howManyWritten() != TEST_12_FLUSH)
     {
          FAILED_TEST("12", "No match between number of tasks then tasks written");
          printf("\tAt this stage, you should have written exactly %d tasks.\n", TEST_12_FLUSH);
          printf("instead - got %d tasks",howManyWritten() );
          exit(-3);
     }

     for ( int i = 0 ; i < maxId ; ++i )
     {
          if ( wasItWritten(i) != 0 )
          {
              FAILED_TEST("12", "All tasks should have been written by now");
              printf("\tSpecifically, task with id %d should have been written\n", i);
              exit(-3);
          }
     }

     for ( int i = 0 ; i < TEST_12_FLUSH ; ++i )
     {
    	 newBuf = (char*)malloc( countDigits(i) + 2 );
    	 sprintf(newBuf,"%d\n", i);
    	 newBufLen = strlen(newBuf);

    	 if ( write2device(newBuf, newBufLen) >= TEST_12_FLUSH )
    	 {
    		 FAILED_TEST("12", "You assign higher id's than you should");
    		 printf("\tAt this stage, id's should be reused");
    		 exit(-3);
    	 }
    	 free(newBuf);
     }

     closedevice();
     wait4close();
     PASSED_TEST("12");
}



void* writeThread ( void* tid )
{
     long a = (long)tid;
     char* newBuf;
     int newBufLen;
     int id;

     for ( int i = 0 ; i < BUF_PER_THREAD ; ++i )
     {
    	 newBuf = (char*)malloc( countDigits(i) + 2 );
    	 sprintf(newBuf,"%d\n", i);
    	 newBufLen = strlen(newBuf);

    	 id =  write2device(newBuf, newBufLen);

    	 if (id == -1)
    	 {
    		 printf("\tThread with id: %ld failed to write to device.\n", a);
    		 printf("\tTest continues...\n");

    	 }
    	 if (test_13_max < id) test_13_max = id;
    	 free(newBuf);
     }

     pthread_exit(NULL);
}




//Advanced testing - multi-threaded run - 1
// forcing to join created threads. This should test your mutexes and cond_vars
void test_13()
{
     remove("./Tests/test_13");
     test_13_max = 0;
     initdevice("./Tests/test_13");

     pthread_t writeThreads[NUM_OF_THREADS];
     for (long i = 0 ; i < NUM_OF_THREADS ; ++i )
     {
          if ( pthread_create( &writeThreads[i], NULL, writeThread, (void*)i) )
          {
              printf("Failed to create thread number %ld\n.", i);
              printf("Test has ended. (Re-run for completion)\n");
              exit(-3);
          }
     }

     for (int i = 0 ; i < NUM_OF_THREADS ; ++i )
     {
          if ( pthread_join( writeThreads[i], NULL ) )
          {
              printf("Failed to join with thread number %d\n.", i);
              printf("Test has ended. Re-run to complete. \n");
              exit(-3);
          }
     }

     closedevice();
     wait4close();

     FILE* f = fopen("./Tests/test_13","r");
     char* newBuf;
     size_t count;

     for (int j = 0 ; j < BUF_PER_THREAD ; ++j )
     {
    	 newBuf = (char*)malloc( countDigits(j) + 2 );
    	 sprintf(newBuf,"%d\n", j);

    	 count = countStringInFile (f, newBuf);
    	 if ( count != NUM_OF_THREADS )
    	 {
    		 FAILED_TEST("13", "The file doesn't contain the requested data");
    		 printf("\tIt should contain %d time(/s) the number %d got %d instead \n" , NUM_OF_THREADS, j,count);
    	 }
    	 free(newBuf);
    	 rewind(f);
     }
     fclose(f);

     closedevice();
     wait4close();
     PASSED_TEST("13");
}


// Advanced testing - write after close
// Close is non-blocking. We check to no new write tasks are accepted once close was set
void test_14()
{
     remove("./Tests/test_14");
     initdevice("./Tests/test_14");
     closedevice();
     if ( write2device ("After closing", strlen("After closing")) != -1 )
     {
          FAILED_TEST("14", "You cannot write a new task after device started closing.");
          exit(-3);
     }


     /* It is possible that you passed the test because the device has finished closing
     * before the call to write2device - thus, you get return value of -1 due to the
      * check if the device is open (i.e. initdevice was called).
      * If you want to make sure that indeed you don't allow writing new tasks after close
     * has been made, insert a long while loop to your 'closedevice' function - this will
     * affectively cause context switch from the 'close' thread, and you can see if you get
     * what you expect.
     */
     wait4close();
     PASSED_TEST("14");
}

/* At the tests you want to check */
int main()
{
	freopen("/dev/null", "w", stderr);
	if ( mkdir("./Tests",S_IRWXU | S_IRWXG | S_IROTH ) )
	{
		if ( errno != EEXIST)
		{
			printf("Failed to create directory for tests. Ended.");
			exit(-3);
		}

	}

	int i =0;

	/* It is not recommended to enable all tests at once */
	while ( i++ < 1000) {

	test_1();
	test_2();
	test_3();
	test_4();
	test_5();
	test_6("test_6_7 is a very simple test", strlen("test_6_7 is a very simple test"), 1);
	test_7();
	test_8();
	test_9();
	test_10();
	test_11();
	test_12();
	test_13();
	test_14();

		printf("--------------------Done section number: %d-----------------------------\n", i);
	}

	return 0;
}


