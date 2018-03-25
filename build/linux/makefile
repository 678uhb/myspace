vars_old := $(.VARIABLES)

cc=g++

cxxflags=-g -pthread -std=c++11 -I../../.. -MMD -MP

src= ../../logger/logger.cpp \
	../../threadpool/threadpool.cpp 

obj=$(patsubst %.cpp,%.o,$(src))

def=$(patsubst %.o,%.d,$(obj))

target=libmyspace.a

all:$(target)

libmyspace.so:$(obj)
	$(cc) $^ -shared -o $@
	chmod +x $@
	mv $@ ../../libs/$@
	
libmyspace.a:$(obj)
	ar rcs $@ $^
	mv $@ ../../libs/$@

.PHONY: test
test:libmyspace.a
	$(cc) $(cxxflags) -o $@ ../../test/main.cpp -g -pthread  -L../../libs -l:libmyspace.a
	chmod +x $@
	mv $@ ../../test/$@
	
%.o:%.cpp
	$(cc) $(cxxflags) -c -o $@ $<

-include $(def) 

.PHONY: info clean
info:
	$(foreach v, $(filter-out $(vars_old) vars_old, $(.VARIABLES)), $(info $(v) = $($(v))))
clean:
	rm -rf $(def)
	rm -rf $(obj)

