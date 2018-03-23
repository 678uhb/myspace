#include "myspace/logger/logger.h"
#include "myspace/strings/strings.h"
#include "myspace/time/time.h"
#include "myspace/path/path.h"
myspace_begin

Logger logger;

void ConsoleSink::put(shared_ptr<LoggerItem> item)
{
	cout << "[" << Time::format(system_clock::to_time_t(item->_time))
		<< "][" << Path::basename(item->_file) << ":" << item->_line << "]:"
		<< item->_content;
	if (item->_content.empty() || item->_content.back() != '\n')
		cout << endl;
}

Logger::Logger()
{
	_sinks.push_back(new_shared<ConsoleSink>());
}

myspace_end
