
#pragma once

#include "myspace/_/include.hpp"
#include "myspace/critical/critical.hpp"
#include "myspace/memory/memory.hpp"
#include "myspace/mutex/mutex.hpp"
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
  void put(shared_ptr<LoggerItem> item);
};

class Logger {
public:
  Logger();

  Logger &setLevel(int lv);

  LoggerLevel getLevel();

  template <class... Targs>
  Logger &printDebug(const char *file, int line, Targs &&... args);

  template <class... Targs>
  Logger &printInfo(const char *file, int line, Targs &&... args);

  template <class... Targs>
  Logger &printWarn(const char *file, int line, Targs &&... args);

  template <class... Targs>
  Logger &printError(const char *file, int line, Targs &&... args);

  template <class... Targs>
  Logger &print(LoggerLevel lv, const char *file, int line, Targs &&... args);

  static Logger &staticInstance();

private:
  LoggerLevel level_ = LoggerLevel::Info;

  deque<shared_ptr<Sink>> sinks_;
};

inline Logger &Logger::staticInstance() {
  static Logger l;
  return l;
}

namespace loggerimpl {

template <class T, class... Targs>
typename enable_if<!is_same<char const *, typename decay<T>::type>::value &&
                       !is_same<char *, typename decay<T>::type>::value,
                   void>::type
format2(stringstream &ss, T &&a, Targs &&... args);

template <class T, class... Targs>
typename enable_if<is_same<char const *, typename decay<T>::type>::value ||
                       is_same<char *, typename decay<T>::type>::value,
                   void>::type
format2(stringstream &ss, T &&a, Targs &&... args);

void format2(stringstream &);

template <class T, class... Ts>
inline void format3(stringstream &ss, const char *ptr, T &&t, Ts &&... ts) {
  ss << t;
  format2(ss, ptr + 2, forward<Ts>(ts)...);
}

inline void format3(stringstream &ss, const char *ptr) { ss << ptr; }

inline void format2(stringstream &) {}

template <class T, class... Targs>
inline
    typename enable_if<!is_same<char const *, typename decay<T>::type>::value &&
                           !is_same<char *, typename decay<T>::type>::value,
                       void>::type
    format2(stringstream &ss, T &&a, Targs &&... args) {
  ss << forward<T>(a);
  format2(ss, forward<Targs>(args)...);
}

template <class T, class... Targs>
inline
    typename enable_if<is_same<char const *, typename decay<T>::type>::value ||
                           is_same<char *, typename decay<T>::type>::value,
                       void>::type
    format2(stringstream &ss, T &&fmt, Targs &&... args) {
  for (const char *ptr = fmt; *ptr; ++ptr) {
    if (ptr[0] == '%' && ptr[1] == 's') {
      format3(ss, ptr, forward<Targs>(args)...);
      return;
    } else {
      ss << ptr[0];
    }
  }
  format2(ss, forward<Targs>(args)...);
}

template <class... Targs> inline string format(Targs &&... args) {
  stringstream ss;
  format2(ss, forward<Targs>(args)...);
  return ss.str();
}

} // namespace loggerimpl

inline void ConsoleSink::put(shared_ptr<LoggerItem> item) {
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

inline Logger::Logger() { sinks_.push_back(new_shared<ConsoleSink>()); }

inline Logger &Logger::setLevel(int lv) {
  if (lv < LoggerLevel::Dev) {
    level_ = LoggerLevel::Dev;
  } else if (lv > LoggerLevel::Error) {
    level_ = LoggerLevel::Error;
  } else {
    level_ = (LoggerLevel)lv;
  }

  return *this;
}

inline LoggerLevel Logger::getLevel() { return level_; }

template <class... Targs>
inline Logger &Logger::printDebug(const char *file, int line,
                                  Targs &&... args) {
  if (level_ > LoggerLevel::Debug)
    return *this;

  return print(LoggerLevel::Debug, file, line, forward<Targs>(args)...);
}

template <class... Targs>
inline Logger &Logger::printInfo(const char *file, int line, Targs &&... args) {
  if (level_ > LoggerLevel::Info)
    return *this;

  return print(LoggerLevel::Info, file, line, forward<Targs>(args)...);
}

template <class... Targs>
inline Logger &Logger::printWarn(const char *file, int line, Targs &&... args) {
  if (level_ > LoggerLevel::Warn)
    return *this;

  return print(LoggerLevel::Warn, file, line, forward<Targs>(args)...);
}

template <class... Targs>
inline Logger &Logger::printError(const char *file, int line,
                                  Targs &&... args) {
  if (level_ > LoggerLevel::Error)
    return *this;

  return print(LoggerLevel::Error, file, line, forward<Targs>(args)...);
}

template <class... Targs>
inline Logger &Logger::print(LoggerLevel lv, const char *file, int line,
                             Targs &&... args) {
  auto item = new_shared<LoggerItem>();
  item->_file = file;
  item->_line = line;
  item->_lv = lv;
  item->_content = loggerimpl::format(forward<Targs>(args)...);

  for (auto sink : sinks_) {
    sink->put(item);
  }
  return *this;
}

MYSPACE_END

#define MYSPACE_SETLOGLEVEL(lv) myspace::Logger::staticInstance().setLevel(lv)

#define MYSPACE_DEV(...)                                                       \
  {                                                                            \
    if (myspace::Logger::staticInstance().getLevel() <=                        \
        myspace::LoggerLevel::Dev) {                                           \
      myspace::Logger::staticInstance().print(LoggerLevel::Dev, __FILE__,      \
                                              __LINE__, ##__VA_ARGS__);        \
    }                                                                          \
  }
#define MYSPACE_DEBUG(...)                                                     \
  {                                                                            \
    if (myspace::Logger::staticInstance().getLevel() <=                        \
        myspace::LoggerLevel::Debug) {                                         \
      myspace::Logger::staticInstance().printDebug(__FILE__, __LINE__,         \
                                                   ##__VA_ARGS__);             \
    }                                                                          \
  }
#define MYSPACE_INFO(...)                                                      \
  {                                                                            \
    if (myspace::Logger::staticInstance().getLevel() <=                        \
        myspace::LoggerLevel::Info) {                                          \
      myspace::Logger::staticInstance().printInfo(__FILE__, __LINE__,          \
                                                  ##__VA_ARGS__);              \
    }                                                                          \
  }
#define MYSPACE_WARN(...)                                                      \
  {                                                                            \
    if (myspace::Logger::staticInstance().getLevel() <=                        \
        myspace::LoggerLevel::Warn) {                                          \
      myspace::Logger::staticInstance().printWarn(__FILE__, __LINE__,          \
                                                  ##__VA_ARGS__);              \
    }                                                                          \
  }
#define MYSPACE_ERROR(...)                                                     \
  {                                                                            \
    if (myspace::Logger::staticInstance().getLevel() <=                        \
        myspace::LoggerLevel::Error) {                                         \
      myspace::Logger::staticInstance().printError(__FILE__, __LINE__,         \
                                                   ##__VA_ARGS__);             \
    }                                                                          \
  }
#define MYSPACE_EXCEPTION() MYSPACE_ERROR(myspace::Exception::dump());

#define MYSPACE_DEBUG_SECONDS(x, ...)                                          \
  MYSPACE_IF_PAST_SECONDS(x) MYSPACE_DEBUG(__VA_ARGS__);
#define MYSPACE_INFO_SECONDS(x, ...)                                           \
  MYSPACE_IF_PAST_SECONDS(x) MYSPACE_INFO(__VA_ARGS__);
#define MYSPACE_WARN_SECONDS(x, ...)                                           \
  MYSPACE_IF_PAST_SECONDS(x) MYSPACE_WARN(__VA_ARGS__);
#define MYSPACE_ERROR_SECONDS(x, ...)                                          \
  MYSPACE_IF_PAST_SECONDS(x) MYSPACE_ERROR(__VA_ARGS__);
