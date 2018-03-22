#include "myspace/logger/logger.h"
#include "myspace/strings/strings.h"
myspace_begin

void ConsoleSink::put(shared_ptr<LoggerItem> item)
{
	cout << "[" << system_clock::to_time_t(item->_time) 
		<< "][" << item->_file << ":" << item->_line << "]:" 
		<< item->_content << endl;
}

myspace_end
