CXX=g++
CFLAGS=-g -O2 -Wall -fPIC -Wno-deprecated 

INC=-I../../include
LIB=-L../../lib -lreactor

all:
	$(CXX) -o lars_reactor_test $(CFLAGS)  lars_reactor.cpp $(INC) $(LIB)

clean:
	-rm -f *.o lars_reactor_test
