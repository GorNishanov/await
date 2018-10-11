CXX=clang++
FLAGS=-O2 -fcoroutines-ts -std=c++2a -stdlib=libc++

a.out:	nanotest.cpp Makefile rng.h naive.h sm.h coro.h
	$(CXX) $(FLAGS) nanotest.cpp -o nanotest
