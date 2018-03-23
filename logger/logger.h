
#pragma once

#include "myspace/config.h"
#include "myspace/critical/critical.h"
#include "myspace/strings/strings.h"
#include "myspace/memory/memory.h"
#include "myspace/mutex/mutex.h"
myspace_begin

struct LoggerItem
{
	int						_line = 0;
	const char*				_file = nullptr;

	string					_content;

	system_clock::time_point _time = system_clock::now();
};

class Sink
{
public:
	virtual ~Sink() {}

	virtual void put(shared_ptr<LoggerItem> item) = 0;
};

class ConsoleSink : public Sink
{
public:
	void put(shared_ptr<LoggerItem> item);
};

class Logger
{
public:
	Logger();

	template<class... Targs>
	Logger& printDebug(const char* file, int line, Targs&&... args)
	{
		return print(file, line, forward<Targs>(args)...);
	}

	template<class... Targs>
	Logger& printInfo(const char* file, int line, Targs&&... args)
	{
		return print(file, line, forward<Targs>(args)...);
	}

	template<class... Targs>
	Logger& printWarn(const char* file, int line, Targs&&... args)
	{
		return print(file, line, forward<Targs>(args)...);
	}

	template<class... Targs>
	Logger& printError(const char* file, int line, Targs&&... args)
	{
		return print(file, line, forward<Targs>(args)...);
	}

private:

	template<class... Targs>
	Logger& print(const char* file, int line, Targs&&... args)
	{
		auto item = new_shared<LoggerItem>();
		item->_file = file;
		item->_line = line;
		item->_content = move(StringStream(forward<Targs>(args)...).str());

		for (auto sink : _sinks)
		{
			sink->put(item);
		}
		return *this;
	}

private:
	deque<shared_ptr<Sink>>					  _sinks;
};

#define debug(...)  printDebug(__FILE__,__LINE__,##__VA_ARGS__)
#define warn(...)	printWarn(__FILE__,__LINE__,##__VA_ARGS__)
#define info(...)	printInfo(__FILE__,__LINE__,##__VA_ARGS__)
#define error(...)  printError(__FILE__,__LINE__,##__VA_ARGS__)

extern Logger logger;

myspace_end