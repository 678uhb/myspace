
#pragma once

#include "myspace/myspace_include.h"
#include "myspace/critical/critical.hpp"
#include "myspace/strings/strings.hpp"
#include "myspace/memory/memory.hpp"
#include "myspace/mutex/mutex.hpp"
#include "myspace/time/time.hpp"
#include "myspace/path/path.hpp"
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
	enum Level
	{
		Debug = 0,
		Info = 1,
		Warn = 2,
		Error = 3,
	};
public:
	Logger()
	{
		_sinks.push_back(new_shared<ConsoleSink>());
	}

	Logger& setLevel(int lv)
	{
		if (lv < 0)
		{
			_level = Level::Debug;
		}
		else if (lv > Level::Error)
		{
			_level = Level::Error;
		}
		else
		{
			_level = (Level)lv;
		}

		return *this;
	}

	Level getLevel()
	{
		return _level;
	}

	template<class... Targs>
	Logger& printDebug(const char* file, int line, Targs&&... args)
	{
		if (_level > Level::Debug) return *this;

		return print(file, line, forward<Targs>(args)...);
	}

	template<class... Targs>
	Logger& printInfo(const char* file, int line, Targs&&... args)
	{
		if (_level > Level::Info) return *this;

		return print(file, line, forward<Targs>(args)...);
	}

	template<class... Targs>
	Logger& printWarn(const char* file, int line, Targs&&... args)
	{
		if (_level > Level::Warn) return *this;

		return print(file, line, forward<Targs>(args)...);
	}

	template<class... Targs>
	Logger& printError(const char* file, int line, Targs&&... args)
	{
		if (_level > Level::Error) return *this;

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

	Level										_level = Level::Info;

	deque<shared_ptr<Sink>>						_sinks;
};


extern Logger logger;


#define my_debug(...)  if(logger.getLevel() <= Logger::Debug  ) {logger.printDebug(__FILE__,__LINE__,##__VA_ARGS__);}
#define my_info(...)	if(logger.getLevel() <= Logger::Info  ){  logger.printInfo(__FILE__,__LINE__,##__VA_ARGS__);}
#define my_warn(...)	if(logger.getLevel() <= Logger::Warn  ){ logger.printWarn(__FILE__,__LINE__,##__VA_ARGS__) ;}
#define my_error(...)  if(logger.getLevel() <= Logger::Error  ){  logger.printError(__FILE__,__LINE__,##__VA_ARGS__);}

myspace_end