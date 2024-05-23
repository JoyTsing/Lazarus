CXX=g++
CFLAGS=-g -O2 -Wall -fPIC -Wno-deprecated 

INC=-I../../include
LIB=-L../../lib -lreactor

all:
	$(CXX) -o server $(CFLAGS)  lars_reactor.cpp $(INC) $(LIB)
	$(CXX) -o client $(CFLAGS)  lars_client.cpp $(INC) $(LIB)

clean:
	-rm -f *.o server client
