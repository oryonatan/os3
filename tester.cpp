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
#include <unistd.h>
#include <stdlib.h>
#include <iosfwd>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <sstream>
#include "outputdevice.h"
#include <stdio.h>
bool BIG_BUFFER = false ;
bool BLOCKING_FLAG = false;
bool FLUSH_FLAG = false ;
bool CLOSE_FLAG = false ;
bool ILLEGAL_FLUSH = false ;
bool WASWRITTEN_FLAG = false;
int id_max=0;
int STATUS=0;
char * OUT1 = "out1.txt";
char * OUT2 = "out2.txt";

//
//int main ()
//{
////	TaskList tl;
//	initdevice("bla.txt");
//	char * buf = "abcdefg\n";
//	for (int var = 0; var < 1000; ++var) {
//		write2device(buf,strlen(buf));
//	}
//	closedevice();
////
//	wait4close();
//	cout<<"\nFIN\n";
//}
int NUMOPS = 0 ;
pthread_t single_thread ;

//write lot of stuff to the device
//if FLUSH_FLAG and ILLEGAL_FLUSH flush iliigal
//else, if FLUSH_FLAG flush
void * mywriter (void * thread_id) {
    sleep(2);//sleep two sec
    for (int i = 0; i < NUMOPS; ++i) {

        //printf ("[DEBUG] CP mywriter: iter num %d out of (%d) iterations\n" , i+1 , NUMOPS) ;

        ////////write lot of stuff to stream
        const std::string once ("writer") ;
        int times = int (fabs(((double)rand()/(double)RAND_MAX) * 50.0)+1) ;
        std::stringstream stream ;
        stream << "Thread " << (long) thread_id;
        stream << " (" << i+1 << "): " ;
        for (int j = 0 ; j < times ; ++j) {
            stream << once << j << " " ;
        }
        if (BIG_BUFFER) {
            static const int extra = 1000 ;
            for (int k = 0 ; k < extra ; ++k) {
                stream << "_BIG_" ;
            }
        }
        stream << std::endl ;
        //////////////////////////////////////////////

        //wrtie the stream to a buffer and then to the device
        int len = stream.str().size() ;
        char * buf = (char *) malloc (len+1) ;
        strcpy (buf , stream.str().c_str()) ;
        int id;
        int num_times=1;
        if(BLOCKING_FLAG){
            num_times=5;
        }
        for(int r=0 ; r<num_times ; r++){
            id = write2device(buf, len);
        }

        STATUS=id;
        if((STATUS)<0 && !WASWRITTEN_FLAG){
            fprintf(stderr,"STATUS=%d\n",STATUS);
        }
        if(id>id_max){
            id_max=id;
        }
        free(buf);
        if (FLUSH_FLAG) {
            if (ILLEGAL_FLUSH) { //TODO, can't it be that id++ still leagal?
                if(WASWRITTEN_FLAG){
                    STATUS = wasItWritten(id_max+1);
                    fprintf(stderr,"STATUS=%d\n",STATUS);
                    return NULL;
                }
                flush2device (id_max+1) ;
            }
            flush2device (id) ;
            if(WASWRITTEN_FLAG){
                STATUS = wasItWritten(id);
                fprintf(stderr,"STATUS=%d\n",STATUS);
            }


        }

    }
        return NULL;
}

int main() {
	STATUS = initdevice (OUT1) ;
	if (STATUS < 0) {
		printf ("[ERROR] Failed to initialize device.\n") ;
		return 1 ;
	}
	fprintf(stderr,"\n5 ");
	NUMOPS = 10 ;
	FLUSH_FLAG = true ;

	pthread_t single_thread ;
	pthread_create (&single_thread , NULL , &mywriter , 0) ;
	void * ignore ;
	pthread_join (single_thread , &ignore) ;

	closedevice() ;

	FLUSH_FLAG = false ;

	return 0 ;
}
