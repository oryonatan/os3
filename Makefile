CFLAGS = -g -Wall -I. -std=c++11


all: liboutputdevice.a 
liboutputdevice.a:  outputdevice.o TaskList.o
	ar rcu liboutputdevice.a outputdevice.o TaskList.o
	ranlib liboutputdevice.a


outputdevice.o: outputdevice.cpp outputdevice.h TaskList.h 
	g++ outputdevice.cpp $(CFLAGS)  -c

TaskList.o: TaskList.cpp TaskList.h
	g++ TaskList.cpp $(CFLAGS) -c



clean:
	rm -rf *.o *.a *~
