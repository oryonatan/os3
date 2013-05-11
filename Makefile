CFLAGS = -g -Wall -I. -std=c++11


all: liboutputdevice.a 
liboutputdevice.a:  outputdevice.o TaskList.o safelocks.o
	ar rcu liboutputdevice.a outputdevice.o TaskList.o safelocks.o
	ranlib liboutputdevice.a


outputdevice.o: outputdevice.cpp outputdevice.h TaskList.h
	g++ outputdevice.cpp $(CFLAGS)  -c

TaskList.o: TaskList.cpp TaskList.h 
	g++ TaskList.cpp $(CFLAGS) -c

safelocks.o: safelocks.cpp safelocks.h
	g++ safelocks.cpp $(CFLAGS) -c


clean:
	rm -rf *.o *.a *~
