vars_old := $(.VARIABLES)

cc=g++

cxxflags=-g -pthread -std=c++11 -I../../.. -MD -MP

linkflags=-pthread

src=\
../../config/config.cc         \
../../coroutine/coroutine.cc   \
../../critical/critical.cc     \
../../error/error.cc           \
../../exception/exception.cc   \
../../listener/listener.cc     \
../../logger/logger.cc         \
../../mutex/mutex.cc           \
../../os/os.cc                 \
../../pack/pack.cc             \
../../path/path.cc             \
../../select/select.cc         \
../../socket/socket.cc         \
../../strings/strings.cc       \
../../threadpool/threadpool.cc \
../../time/time.cc             \
../../uuid/uuid.cc             \
../../wav/wav.cc


obj=$(patsubst %.cc,%.o,$(src))

def=$(patsubst %.o,%.d,$(obj))

target=../../libs/libmyspace.a

all:$(target)

../../libs/libmyspace.a:$(obj)
	ar -crs $@  $^

%.o:%.cc
	$(cc) $(cxxflags) -c -o $@ $<

-include $(def) 

.PHONY: info clean
info:
	$(foreach v, $(filter-out $(vars_old) vars_old, $(.VARIABLES)), $(info $(v) = $($(v))))
clean:
	rm -rf $(def)
	rm -rf $(obj)

