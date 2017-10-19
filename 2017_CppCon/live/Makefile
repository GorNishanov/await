.PHONY: all clean

BIN=bin/myserver bin/myclient bin/first bin/easy bin/hard1 bin/hard2 \
	bin/over1 bin/over2 bin/stop1

all: $(BIN)

clean:
	rm $(BIN)

INC=-I/home/gor/src/networking-ts-impl/include

BOOST=-I/home/gor/src/boost_1_65_1 -lboost_system -lboost_thread

WARN= -Wall -Wno-unused-local-typedef -Wno-unused-private-field

CFLAGS=-std=c++1z -fcoroutines-ts -stdlib=libc++ -pthreads -ferror-limit=1 $(INC)
CC = clang++

bin/myclient: myclient.cpp handler_allocator.hpp
	$(CC) myclient.cpp -O2 -o bin/myclient $(CFLAGS)

bin/myserver: myserver.cpp
	$(CC) myserver.cpp -O2 -o bin/myserver $(CFLAGS)

bin/first: first.cpp
	$(CC) first.cpp -O2 -o bin/first $(CFLAGS)

bin/easy: easy.cpp
	$(CC) easy.cpp -g -O2 -o bin/easy $(CFLAGS) $(BOOST)

bin/hard1: hard1.cpp future_adapter.h
	$(CC) hard1.cpp -O2 -o bin/hard1 $(CFLAGS) $(BOOST)

bin/hard2: hard2.cpp await_adapters.h
	$(CC) hard2.cpp -O2 -o bin/hard2 $(CFLAGS)

bin/over1: over1.cpp
	$(CC) over1.cpp -O2 -o bin/over1 $(CFLAGS) $(BOOST)

bin/over2: over2.cpp
	$(CC) over2.cpp -O2 -o bin/over2 $(CFLAGS) $(BOOST)

bin/stop1: stop1.cpp
	$(CC) stop1.cpp -O2 -o bin/stop1 $(CFLAGS)
