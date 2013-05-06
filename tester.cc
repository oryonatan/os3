///*
//
// A tester for ex3 OS - Spring 2012.
//
//*/
//
//
//// includes:
//#include <pthread.h>
//#include <unistd.h>
//#include <cstdlib>
//#include <cstdio>
//#include <cstring>
//#include <cmath>
//#include <sys/time.h>
//#include <sys/resource.h>
//#include <sys/types.h>
//#include <signal.h>
//#include <string>
//#include <sstream>
//#include "outputdevice.h"
//#include <stdio.h>
//
//
//// globals:
//char * OUT1;// = "out1.txt";
//char * OUT2;// = "out2.txt";
//
//int NUMOPS = 0 ;
//bool BLOCKING_FLAG = false;
//bool FLUSH_FLAG = false ;
//bool CLOSE_FLAG = false ;
//bool ILLEGAL_FLUSH = false ;
//bool BIG_BUFFER = false ;
//bool WASWRITTEN_FLAG = false;
//int id_max=0;
//int STATUS=0;
//// tests:
//int test1() ;
//int test2() ;
//int test3 (bool init , bool preClose) ;
//int test5() ;
//int test6() ;
//int test7() ;
//int test8() ;
//int test9 (bool big_buf,bool to_kill) ;
//int test10() ;
//int test11() ;
//int test12() ;
//int test13() ;
//int test14() ;
//int test15() ;
//int test16() ;
//
//// main:
//
////return value:
////1  -testing failed (could not initialize the device)
////0  -testing succeeded and testing status is 0
////-1 -testing succeeded and testing status is negative (for example when causing error by porpuse)
//int main (int argc , char ** argv) {
//    int ret_val = 0;
//    if (argc != 2) {
//        printf ("[ERROR] Usage: %s <test_num>\n" , argv[0]) ;
//        exit (-1) ;
//    }
//
//    int test_num = atoi (argv[1]) ;
//    if (test_num == 0) {
//        printf ("[ERROR] %s called with non-numeric test number.\n" ,
//                argv[0]) ;
//        exit (-1) ;
//    }
//    OUT1 = (char*) malloc(strlen("out1.txt"));
//    strcpy(OUT1,"out1.txt");
//    OUT2 = (char*) malloc(strlen("out2.txt"));
//    strcpy(OUT2,"out2.txt");
//
//
//
//    // Set random seed:
//    srand ((unsigned)time(NULL)) ;
//
//    printf ("[INFO] Running test %d ...\n" , test_num) ;
//    fflush (stdout) ;
//
//    switch (test_num) {
//      case 1:
//          ret_val = test1(); break;// intialize and close and that's it
//      case 2:
//          ret_val = test2(); break;
//      case 3:
//          ret_val = test3 (true , false); break;  //with initialization, int test3(bool initialize ,bool preClose) //preClose closese imidiatly after init
//      case 5:
//          ret_val = test5(); break;
//      case 6:
//          ret_val = test6(); break;
//      case 7:
//          ret_val = test7(); break;
//      case 8:
//          ret_val = test8(); break;
//      case 9:
//          ret_val = test9 (false,false); break; // no big buffers
//      case 10:
//          ret_val = test10(); break;
//      case 11:
//          ret_val = test11(); break;
//      case 12:
//          ret_val = test12(); break;
//      case 13:
//          ret_val = test13(); break;
//      case 14:
//          ret_val = test14(); break;
//      case 15:
//          ret_val = test15(); break;
//      case 16:
//          ret_val = test16(); break;
//
//      default:
//          printf ("[ERROR] Test %d is not handled.\n" , test_num) ;
//          ret_val = -1;
//    } // switch (test_num)
//
//    free(OUT1);
//    free(OUT2);
//
//    pthread_exit(NULL);
//
//
//    printf("yofi");
//    return ret_val;
//    /*fprintf(stderr,"\ntest_num=%d,STATUS=%d\n",test_num,STATUS);
//    fflush (stderr) ;
//    if(ret_val==1){
//        return 1;
//    }
//    if(STATUS >= 0){
//        return 0;
//    }
//    return STATUS;*/
//}
//
//
//// functions:
//
////write 3 different writings to the device:
////
//// write "Thread before #sec"  (BUT NOT FLUSH)
//// write "Thread, after-before" (FLUSH IF FLUSH_FLAG) and overide id from previous
//// write "Thread thread_id flush, afterFlush-before" (BUT NOT FLUSH) and overide id from previous
////return NULL
//void * mytimewriter (void * thread_id) {
//
//	std::stringstream stream, stream2, stream3 ;
//	timeval a;
//	unsigned long long int diff, useconds_before,useconds_after,useconds_after_flush;
//
//
//
//        //-------------------------------------------------------------
//        //write 2 device: "Thread before #sec"  (BUT NOT FLUSH)
//        //-----------------------------------
//
//	gettimeofday(&a, NULL);
//	useconds_before = a.tv_sec*1000 + a.tv_usec;
//
//	stream << "Thread " << (long) thread_id;
//	stream << "\tbefore\t" << useconds_before;
//	stream << std::endl ;
//
//	int len = stream.str().size() ;
//        char * buf = (char *) malloc (len+1) ;
//	strcpy (buf , stream.str().c_str()) ;
//	int id = write2device(buf, len);
//        if(id>id_max){
//            id_max=id;
//        }
//	free(buf);
//	//--------------------------------------------------------------
//
//
//
//	//--------------------------------------------------------------
//        //write "Thread, after-before" (BUT NOT FLUSH) and overide id from previous
//	//--------------------------------------------------------------
//	gettimeofday(&a, NULL);
//	useconds_after = a.tv_sec*1000 + a.tv_usec;
//	diff = useconds_after < useconds_before ? 0 : useconds_after-useconds_before;
//
//	stream2 << "Thread " << (long) thread_id;
//	stream2 << "\twrite\t" << diff;
//	stream2 << std::endl ;
//
//	len = stream2.str().size() ;
//        buf = (char *) malloc (len+1) ;
//        strcpy (buf , stream2.str().c_str()) ;
//        id = write2device(buf, len);
//         if(id>id_max){
//             id_max=id;
//        }
//
//        free(buf);
//	//--------------------------------------------------------------
//
//	if (FLUSH_FLAG) {
//            flush2device (id); //block until the spicified id has been written to the file
//	}
//
//	//--------------------------------------------------------------
//        // write "Thread thread_id flush, afterFlush-before" (BUT NOT FLUSH) and overide id from previous
//	//--------------------------------------------------------------
//
//	gettimeofday(&a, NULL);
//        useconds_after_flush = a.tv_sec*1000 + a.tv_usec;
//	diff = useconds_after_flush < useconds_before ? 0 : useconds_after_flush-useconds_before;
//
//        stream3 << "Thread " << (long) thread_id;
//        stream3 << "\tflush\t" << diff;
//        stream3 << std::endl ;
//
//
//	len = stream3.str().size() ;
//	buf = (char *) malloc (len+1) ;
//	strcpy (buf , stream3.str().c_str()) ;
//	id = write2device(buf, len);
//        if(id>id_max){
//            id_max=id;
//        }
//        STATUS = id;
//	free(buf);
//	//--------------------------------------------------------------
//	return NULL;
//}
//
////write lot of stuff to the device
////if FLUSH_FLAG and ILLEGAL_FLUSH flush iliigal
////else, if FLUSH_FLAG flush
//void * mywriter (void * thread_id) {
//    sleep(2);//sleep two sec
//    for (int i = 0; i < NUMOPS; ++i) {
//
//        //printf ("[DEBUG] CP mywriter: iter num %d out of (%d) iterations\n" , i+1 , NUMOPS) ;
//
//        ////////write lot of stuff to stream
//        const std::string once ("writer") ;
//        int times = int (fabs(((double)rand()/(double)RAND_MAX) * 50.0)+1) ;
//        std::stringstream stream ;
//        stream << "Thread " << (long) thread_id;
//        stream << " (" << i+1 << "): " ;
//        for (int j = 0 ; j < times ; ++j) {
//            stream << once << j << " " ;
//        }
//        if (BIG_BUFFER) {
//            static const int extra = 1000 ;
//            for (int k = 0 ; k < extra ; ++k) {
//                stream << "_BIG_" ;
//            }
//        }
//        stream << std::endl ;
//        //////////////////////////////////////////////
//
//        //wrtie the stream to a buffer and then to the device
//        int len = stream.str().size() ;
//        char * buf = (char *) malloc (len+1) ;
//        strcpy (buf , stream.str().c_str()) ;
//        int id;
//        int num_times=1;
//        if(BLOCKING_FLAG){
//            num_times=5;
//        }
//        for(int r=0 ; r<num_times ; r++){
//            id = write2device(buf, len);
//        }
//
//        STATUS=id;
//        if((STATUS)<0 && !WASWRITTEN_FLAG){
//            fprintf(stderr,"STATUS=%d\n",STATUS);
//        }
//        if(id>id_max){
//            id_max=id;
//        }
//        free(buf);
//        if (FLUSH_FLAG) {
//            if (ILLEGAL_FLUSH) { //TODO, can't it be that id++ still leagal?
//                if(WASWRITTEN_FLAG){
//                    STATUS = wasItWritten(id_max+1);
//                    fprintf(stderr,"STATUS=%d\n",STATUS);
//                    return NULL;
//                }
//                flush2device (id_max+1) ;
//            }
//            flush2device (id) ;
//            if(WASWRITTEN_FLAG){
//                STATUS = wasItWritten(id);
//                fprintf(stderr,"STATUS=%d\n",STATUS);
//            }
//
//
//        }
//
//    }
//        return NULL;
//}
//
////cause an error in purpuse
//void * myflusher (void * thread_id) {
//
//	sleep(1);
//	fflush(stdout);
//
//	if (CLOSE_FLAG) {
//		fflush(stdout);
//		closedevice() ;
//		wait4close();
//	}
//
//	fflush(stdout);
//	flush2device (0) ;
//
//	return NULL;
//}
//
/////////////////////////////
//// Init the device and then close it:
/////////////////////////////
//int test1() {
//
//    STATUS = initdevice (OUT1) ;
//    if (STATUS < 0) {
//        printf ("[ERROR] Failed to initialize device test1.\n") ;
//        return 1 ;
//    }
//    fprintf(stderr,"\n1 ");
//    //fprintf(stderr,"\ntest_num=1,STATUS=%d\n",STATUS);
//    closedevice() ;
//
//    printf ("[DEBUG] CP 1\n") ;
//
//    return 0 ;
//}
/////////////////////////////
//// Double init the device:q
/////////////////////////////
//int test2() {
//    fprintf(stderr,"\n2 ");
//    STATUS = initdevice (OUT1) ;
//    if (STATUS < 0) {
//        printf ("[ERROR] Failed to initialize device.\n") ;
//        return 1 ;
//    }
//    closedevice();
//    wait4close();
//    printf("device closed and about to be open again\n");
//
//    STATUS = initdevice (OUT1) ;
//
//    if (STATUS < 0) {
//        printf ("[ERROR] Failed to re-initialize device.\n") ;
//        return 1 ;
//    }
//
//    fflush(stdout);
//
//    closedevice();
//    wait4close();
//    return 0 ;
//}
/////////////////////////////
////run an single thread to execute mywriter
////if preClose then close imidiatly after init (which should cause an error)
/////////////////////////////
//int test3 (bool initialize , bool preClose) {
//	if (initialize) {
//		STATUS = initdevice (OUT1) ;
//		if (STATUS < 0) {
//			printf ("[ERROR] Failed to initialize device.\n") ;
//			return 1 ;
//		}
//	}
//
//	if (preClose) {
//            closedevice() ;
//	}
//        if(initialize && (! preClose)){
//            fprintf(stderr,"\n3 ");
//        }
//	pthread_t single_thread ;
//	NUMOPS = 1 ;  // single op
//        void * ignore ;
//        pthread_create (&single_thread , NULL , &mywriter , 0) ;
//
//        /**
//        The  pthread_join()  function  shall  suspend  execution of the calling thread until the target thread terminates, unless the target thread has
//            already  terminated.  the value passed to  pthread_exit()by  the terminating thread shall be made available in value_ptr.
//        **/
//	pthread_join (single_thread , &ignore) ;
//
//	closedevice() ;
//	return 0 ;
//}
//
/////////////////////////////
//// A single thread performs a series of write and flush :
/////////////////////////////
//int test5() {
//	STATUS = initdevice (OUT1) ;
//	if (STATUS < 0) {
//		printf ("[ERROR] Failed to initialize device.\n") ;
//		return 1 ;
//	}
//	fprintf(stderr,"\n5 ");
//	NUMOPS = 10 ;
//	FLUSH_FLAG = true ;
//
//	pthread_t single_thread ;
//	pthread_create (&single_thread , NULL , &mywriter , 0) ;
//	void * ignore ;
//	pthread_join (single_thread , &ignore) ;
//
//	closedevice() ;
//
//	FLUSH_FLAG = false ;
//
//	return 0 ;
//}
//
/////////////////////////////
//// Close and then flush:
/////////////////////////////
//int test6() {
//	STATUS = initdevice (OUT1) ;
//	if (STATUS < 0) {
//		printf ("[ERROR] Failed to initialize device.\n") ;
//		return 1 ;
//	}
//	fprintf(stderr,"\n6 ");
//	NUMOPS = 1 ;
//	CLOSE_FLAG = true ;
//
//	pthread_t single_thread ;
//	pthread_create (&single_thread , NULL , &myflusher , 0) ;
//	void * ignore ;
//	pthread_join (single_thread , &ignore) ;
//
//	CLOSE_FLAG = false ;
//
//	return 0 ;
//}
//
/////////////////////////////
////int test3(int initialize,int preClose)
////if preClose then close imidiatly after init (which should cause an error)
/////////////////////////////
//// Write to an uninitialized device:
/////////////////////////////
//int test7() {
//    fprintf(stderr,"\n7 ");
//    return test3 (false , false) ;
//}
//
/////////////////////////////
//// Illegal flush: (flush an illegal id)
/////////////////////////////
//int test8() {
//
//
//	STATUS = initdevice (OUT1) ;
//	if (STATUS < 0) {
//		printf ("[ERROR] Failed to initialize device.\n") ;
//		return 1 ;
//	}
//	fprintf(stderr,"\n8 ");
//	NUMOPS = 1 ;
//	FLUSH_FLAG = true ;
//	ILLEGAL_FLUSH = true ;
//
//	pthread_t single_thread ;
//	pthread_create (&single_thread , NULL , &mywriter , 0) ;
//	void * ignore ;
//	pthread_join (single_thread , &ignore) ;
//
//	FLUSH_FLAG = false ;
//	ILLEGAL_FLUSH = false ;
//
//	closedevice() ;
//	fprintf<< "test8 success";
//	return 0 ;
//}
//
/////////////////////////////
//// Run the device with a number of threads:
/////////////////////////////
//int test9 (bool big_buf,bool to_kill) {
//	STATUS = initdevice (OUT2) ;
//	if (STATUS < 0) {
//		printf ("[ERROR] Failed to initialize device.\n") ;
//		return 1 ;
//	}
//
//	if (big_buf) {
//		BIG_BUFFER = true ;
//	}
//
//        else{
//            fprintf(stderr,"\n9 ");
//        }
//	NUMOPS = 30 ;
//	const int NUMTHREADS = 10 ;
//	pthread_t producers[NUMTHREADS];
//
//
//        int* id= (int*) malloc(sizeof(int));
//	for (int i = 0; i < NUMTHREADS; i++) {
//            *id = i+1;
//            pthread_create(&producers[i], NULL, &mywriter, (void *)(id));
//	}
//
//	void * ignore ;
//	for (int i = 0; i < NUMTHREADS; i++) {
//            pthread_join(producers[i], &ignore);
//	}
//	if(to_kill){
//            kill(getpid(),SIGTERM);
//        }
//        //fprintf(stderr,"got here\n");
//	closedevice(); wait4close();
//	free(id);
//	return(0);
//}
//
/////////////////////////////
//// same as teset9 but with very big buffers
/////////////////////////////
//int test10() {
//	fprintf(stderr,"\n10 ");
//	return test9 (true,false) ;
//}
//
/////////////////////////////
////test3 (true , true) ;
//// init, close and then write
/////////////////////////////
//int test11() {
//	fprintf(stderr,"\n11 ");
//	return test3 (true , true) ;
//}
//
/////////////////////////////
////create many threads all running mytimewriter on out1.txt without flush
/////////////////////////////
//int test12() {
//	STATUS = initdevice (OUT1);
//	if (STATUS < 0) {
//		printf ("[ERROR] Failed to initialize device.\n") ;
//		return 1 ;
//	}
//        if(!FLUSH_FLAG){
//            fprintf(stderr,"\n12 ");
//        }
//	const int NUMTHREADS = 100 ;
//	pthread_t producers[NUMTHREADS];
//
//        int* id= (int*) malloc(sizeof(int));
//	for (int i = 0; i < NUMTHREADS; i++) {
//            *id=i+1;
//            pthread_create(&producers[i], NULL, &mytimewriter, (void *)id);
//	}
//
//	void * ignore ;
//	for (int i = 0; i < NUMTHREADS; i++) {
//		pthread_join(producers[i], &ignore);
//	}
//
//	closedevice(); wait4close();
//        free(id);
//	return 0;
//}
//
/////////////////////////////
////create many threads all running mytimewriter on out1.txt with flush
/////////////////////////////
//int test13() {
//	FLUSH_FLAG = true;
//        fprintf(stderr,"\n13 ");
//	int status = test12();
//	FLUSH_FLAG = false;
//	return status;
//}
//
/////////////////////////////
//// test wasItWritten for illegal id
/////////////////////////////
//int test14(){
//
//    STATUS = initdevice (OUT1) ;
//    if (STATUS < 0) {
//        printf ("[ERROR] Failed to initialize device.\n") ;
//        return 1 ;
//    }
//    fprintf(stderr,"\n14 ");
//    NUMOPS = 1 ;
//
//    WASWRITTEN_FLAG = true;
//    FLUSH_FLAG = true;
//    ILLEGAL_FLUSH = true ;
//
//    pthread_t single_thread ;
//    pthread_create (&single_thread , NULL , &mywriter , 0) ;
//    void * ignore ;
//    pthread_join (single_thread , &ignore) ;
//
//    FLUSH_FLAG = false ;
//    ILLEGAL_FLUSH = false ;
//    WASWRITTEN_FLAG = false;
//
//    closedevice() ;
//
//    return 0 ;
//}
//
/////////////////////////////
//// test wasItWritten for llegal
/////////////////////////////
//int test15(){
//
//    STATUS = initdevice (OUT1) ;
//    if (STATUS < 0) {
//        printf ("[ERROR] Failed to initialize device.\n") ;
//        return 1 ;
//    }
//    fprintf(stderr,"\n15 ");
//    NUMOPS = 1 ;
//
//    FLUSH_FLAG = true;
//    WASWRITTEN_FLAG = true;
//
//    pthread_t single_thread ;
//    pthread_create (&single_thread , NULL , &mywriter , 0) ;
//    void * ignore ;
//    pthread_join (single_thread , &ignore) ;
//
//    WASWRITTEN_FLAG = true;
//    FLUSH_FLAG = false ;
//
//
//    closedevice() ;
//
//    return 0 ;
//}
//
/////////////////////////////
//// test blocking , run test9 with big buffers, then kill the program after all threads finish and
//// chck that some stuff was not written
/////////////////////////////
//int test16() {
//	fprintf(stderr,"\n16 ");
//	BLOCKING_FLAG = true;
//        return test9 (false,true) ;
//}
