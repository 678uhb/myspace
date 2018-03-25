
#pragma once

#include "myspace/config.hpp"
#include "myspace/critical/critical.hpp"
#include "myspace/strings/strings.hpp"
#include "myspace/memory/memory.hpp"
#include "myspace/mutex/mutex.hpp"
#include "myspace/time/time.hpp"
#include "myspace/path/path.hpp"
MYSPACE_BEGIN

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
	void put(shared_ptr<LoggerItem> item)
	{
		cout << "[" << Time::format(system_clock::to_time_t(item->_time))
			<< "][" << Path::basename(item->_file) << ":" << item->_line << "]:"
			<< item->_content;
		if (item->_content.empty() || item->_content.back() != '\n')
			cout << endl;
	}
};

class Logger
{
public:
	Logger()
	{
		_sinks.push_back(new_shared<ConsoleSink>());
	}

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

MYSPACE_END