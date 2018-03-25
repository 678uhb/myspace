vars_old := $(.VARIABLES)

cc=g++

cxxflags=-g -pthread -std=c++11 -I../../.. -MMD -MP

linkflags=-g -pthread

src= ../../logger/logger.cpp \
	../../threadpool/threadpool.cpp 

obj=$(patsubst %.cpp,%.o,$(src))

def=$(patsubst %.o,%.d,$(obj))

target=libmyspace.so

all:$(target)

libmyspace.so:$(obj)
	$(cc) $^ -shared -o $@
	cp $@ ../../libs/libmyspace.so
	cp $@ ../../test/libmyspace.so

.PHONY: test
test:libmyspace.so
	$(cc) $(cxxflags) -o $@ ../../test/main.cpp $(linkflags) -L. -lmyspace -Wl,-rpath,./
	mv $@ ../../test/test
	chmod +x ../../test/test
	
%.o:%.cpp
	$(cc) $(cxxflags) -fPIC -c -o $@ $<

-include $(def) 

.PHONY: info clean
info:
	$(foreach v, $(filter-out $(vars_old) vars_old, $(.VARIABLES)), $(info $(v) = $($(v))))
clean:
	rm -rf $(def)
	rm -rf $(obj)

