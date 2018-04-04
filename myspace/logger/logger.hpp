
#pragma once

#include "myspace/critical/critical.hpp"
#include "myspace/memory/memory.hpp"
#include "myspace/mutex/mutex.hpp"
#include "myspace/myspace_include.h"
#include "myspace/path/path.hpp"
#include "myspace/strings/strings.hpp"
#include "myspace/time/time.hpp"
MYSPACE_BEGIN

enum LoggerLevel {
  Dev = 0,
  Debug,
  Info,
  Warn,
  Error,
};

struct LoggerItem {
  int _line = 0;

  int _lv;

  const char *_file = nullptr;

  string _content;

  system_clock::time_point _time = system_clock::now();
};

class Sink {
public:
  virtual ~Sink() {}

  virtual void put(shared_ptr<LoggerItem> item) = 0;
};

class ConsoleSink : public Sink {
public:
  void put(shared_ptr<LoggerItem> item) {
    MYSPACE_SYNCHRONIZED {
      cout << "[";
      switch (item->_lv) {
      case LoggerLevel::Dev:
        cout << "dev";
        break;
      case LoggerLevel::Debug:
        cout << "debug";
        break;
      case LoggerLevel::Info:
        cout << "info";
        break;
      case LoggerLevel::Warn:
        cout << "warn";
        break;
      case LoggerLevel::Error:
        cout << "error";
        break;
      }
      cout << "]";

      cout << "[" << Time::format(system_clock::to_time_t(item->_time)) << "]["
           << Path::basename(item->_file) << ":" << item->_line
           << "]:" << item->_content;
      if (item->_content.empty() || item->_content.back() != '\n')
        cout << endl;
    }
  }
};

class Logger {
public:
  Logger() { _sinks.push_back(new_shared<ConsoleSink>()); }

  Logger &setLevel(int lv) {
    if (lv < LoggerLevel::Dev) {
      _level = LoggerLevel::Dev;
    } else if (lv > LoggerLevel::Error) {
      _level = LoggerLevel::Error;
    } else {
      _level = (LoggerLevel)lv;
    }

    return *this;
  }

  LoggerLevel getLevel() { return _level; }

  template <class... Targs>
  Logger &printDebug(const char *file, int line, Targs &&... args) {
    if (_level > LoggerLevel::Debug)
      return *this;

    return print(LoggerLevel::Debug, file, line, forward<Targs>(args)...);
  }

  template <class... Targs>
  Logger &printInfo(const char *file, int line, Targs &&... args) {
    if (_level > LoggerLevel::Info)
      return *this;

    return print(LoggerLevel::Info, file, line, forward<Targs>(args)...);
  }

  template <class... Targs>
  Logger &printWarn(const char *file, int line, Targs &&... args) {
    if (_level > LoggerLevel::Warn)
      return *this;

    return print(LoggerLevel::Warn, file, line, forward<Targs>(args)...);
  }

  template <class... Targs>
  Logger &printError(const char *file, int line, Targs &&... args) {
    if (_level > LoggerLevel::Error)
      return *this;

    return print(LoggerLevel::Error, file, line, forward<Targs>(args)...);
  }

  template <class... Targs>
  Logger &print(LoggerLevel lv, const char *file, int line, Targs &&... args) {
    auto item = new_shared<LoggerItem>();
    item->_file = file;
    item->_line = line;
    item->_lv = lv;
    item->_content = move(StringStream(forward<Targs>(args)...).str());

    for (auto sink : _sinks) {
      sink->put(item);
    }
    return *this;
  }

private:
  LoggerLevel _level = LoggerLevel::Info;

  deque<shared_ptr<Sink>> _sinks;
};

MYSPACE_API extern Logger logger;

MYSPACE_END

#define MYSPACE_DEV(...)                                                       \
  {                                                                            \
    if (myspace::logger.getLevel() <= myspace::LoggerLevel::Dev) {             \
      myspace::logger.print(LoggerLevel::Dev, __FILE__, __LINE__,              \
                            ##__VA_ARGS__);                                    \
    }                                                                          \
  }
#define MYSPACE_DEBUG(...)                                                     \
  {                                                                            \
    if (myspace::logger.getLevel() <= myspace::LoggerLevel::Debug) {           \
      myspace::logger.printDebug(__FILE__, __LINE__, ##__VA_ARGS__);           \
    }                                                                          \
  }
#define MYSPACE_INFO(...)                                                      \
  {                                                                            \
    if (myspace::logger.getLevel() <= myspace::LoggerLevel::Info) {            \
      myspace::logger.printInfo(__FILE__, __LINE__, ##__VA_ARGS__);            \
    }                                                                          \
  }
#define MYSPACE_WARN(...)                                                      \
  {                                                                            \
    if (myspace::logger.getLevel() <= myspace::LoggerLevel::Warn) {            \
      myspace::logger.printWarn(__FILE__, __LINE__, ##__VA_ARGS__);            \
    }                                                                          \
  }
#define MYSPACE_ERROR(...)                                                     \
  {                                                                            \
    if (myspace::logger.getLevel() <= myspace::LoggerLevel::Error) {           \
      myspace::logger.printError(__FILE__, __LINE__, ##__VA_ARGS__);           \
    }                                                                          \
  }

#define MYSPACE_DEBUG_SECONDS(x, ...)                                          \
  MYSPACE_IF_PAST_SECONDS(x) MYSPACE_DEBUG(__VA_ARGS__);
#define MYSPACE_INFO_SECONDS(x, ...)                                           \
  MYSPACE_IF_PAST_SECONDS(x) MYSPACE_INFO(__VA_ARGS__);
#define MYSPACE_WARN_SECONDS(x, ...)                                           \
  MYSPACE_IF_PAST_SECONDS(x) MYSPACE_WARN(__VA_ARGS__);
#define MYSPACE_ERROR_SECONDS(x, ...)                                          \
  MYSPACE_IF_PAST_SECONDS(x) MYSPACE_ERROR(__VA_ARGS__);
