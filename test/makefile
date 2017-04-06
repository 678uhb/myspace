
cc=g++
cxxflags=-std=c++11 -pthread -g -Wall -I..
header=../this_space.hpp
target=test-echo


all:$(target)
test-echo:echo.cpp $(header)
	$(cc) $(cxxflags) $< -o $@
clean:
	rm -f $(target)
