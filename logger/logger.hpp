
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/critical/critical.hpp"
#include "myspace/memory/memory.hpp"
#include "myspace/mutex/mutex.hpp"
#include "myspace/path/path.hpp"
#include "myspace/strings/strings.hpp"
#include "myspace/time/time.hpp"
MYSPACE_BEGIN

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

#define MYSPACE_DEV_EXCEPTION() MYSPACE_DEV(myspace::Exception::dump())
#define MYSPACE_DEBUG_EXCEPTION() MYSPACE_DEBUG(myspace::Exception::dump())
#define MYSPACE_INFO_EXCEPTION() MYSPACE_INFO(myspace::Exception::dump())
#define MYSPACE_WARN_EXCEPTION() MYSPACE_WARN(myspace::Exception::dump())
#define MYSPACE_ERROR_EXCEPTION() MYSPACE_ERROR(myspace::Exception::dump())
#define MYSPACE_DEV_RETHROW_EX(except)                                         \
  {                                                                            \
    auto err = Exception::dump();                                              \
    MYSPACE_DEV(err);                                                          \
    MYSPACE_THROW_EX(except, err);                                             \
  }

#define MYSPACE_DEBUG_SECONDS(x, ...)                                          \
  MYSPACE_IF_PAST_SECONDS(x) MYSPACE_DEBUG(__VA_ARGS__);
#define MYSPACE_INFO_SECONDS(x, ...)                                           \
  MYSPACE_IF_PAST_SECONDS(x) MYSPACE_INFO(__VA_ARGS__);
#define MYSPACE_WARN_SECONDS(x, ...)                                           \
  MYSPACE_IF_PAST_SECONDS(x) MYSPACE_WARN(__VA_ARGS__);
#define MYSPACE_ERROR_SECONDS(x, ...)                                          \
  MYSPACE_IF_PAST_SECONDS(x) MYSPACE_ERROR(__VA_ARGS__);

enum LoggerLevel {
  Dev = 0,
  Debug,
  Info,
  Warn,
  Error,
};

struct LoggerItem {
  int line_ = 0;
  int lv_;
  const char *file_ = nullptr;
  std::string content_;
  std::chrono::system_clock::time_point time_ =
      std::chrono::system_clock::now();
};

class Sink {
public:
  virtual ~Sink() {}
  virtual void put(std::shared_ptr<LoggerItem> item) = 0;
};

class ConsoleSink : public Sink {
public:
  void put(std::shared_ptr<LoggerItem> item);
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

  std::deque<std::shared_ptr<Sink>> sinks_;
};

inline Logger &Logger::staticInstance() {
  static Logger l;
  return l;
}

namespace loggerimpl {

template <class T, class... Targs>
typename std::enable_if<
    !std::is_same<char const *, typename std::decay<T>::type>::value &&
        !std::is_same<char *, typename std::decay<T>::type>::value,
    void>::type
format2(std::stringstream &ss, T &&a, Targs &&... args);

template <class T, class... Targs>
typename std::enable_if<
    std::is_same<char const *, typename std::decay<T>::type>::value ||
        std::is_same<char *, typename std::decay<T>::type>::value,
    void>::type
format2(std::stringstream &ss, T &&a, Targs &&... args);

void format2(std::stringstream &);

template <class T, class... Ts>
inline void format3(std::stringstream &ss, const char *ptr, T &&t,
                    Ts &&... ts) {
  ss << t;
  format2(ss, ptr + 2, std::forward<Ts>(ts)...);
}

inline void format3(std::stringstream &ss, const char *ptr) { ss << ptr; }

inline void format2(std::stringstream &) {}

template <class T, class... Targs>
inline typename std::enable_if<
    !std::is_same<char const *, typename std::decay<T>::type>::value &&
        !std::is_same<char *, typename std::decay<T>::type>::value,
    void>::type
format2(std::stringstream &ss, T &&a, Targs &&... args) {
  ss << std::forward<T>(a);
  format2(ss, std::forward<Targs>(args)...);
}

template <class T, class... Targs>
inline typename std::enable_if<
    std::is_same<char const *, typename std::decay<T>::type>::value ||
        std::is_same<char *, typename std::decay<T>::type>::value,
    void>::type
format2(std::stringstream &ss, T &&fmt, Targs &&... args) {
  for (const char *ptr = fmt; *ptr; ++ptr) {
    if (ptr[0] == '%' && ptr[1] == 's') {
      format3(ss, ptr, std::forward<Targs>(args)...);
      return;
    } else {
      ss << ptr[0];
    }
  }
  format2(ss, std::forward<Targs>(args)...);
}

template <class... Targs> inline std::string format(Targs &&... args) {
  std::stringstream ss;
  format2(ss, std::forward<Targs>(args)...);
  return ss.str();
}

} // namespace loggerimpl

inline void ConsoleSink::put(std::shared_ptr<LoggerItem> item) {
  MYSPACE_SYNCHRONIZED {
    std::cout << "[";
    switch (item->lv_) {
    case LoggerLevel::Dev:
      std::cout << "dev";
      break;
    case LoggerLevel::Debug:
      std::cout << "debug";
      break;
    case LoggerLevel::Info:
      std::cout << "info";
      break;
    case LoggerLevel::Warn:
      std::cout << "warn";
      break;
    case LoggerLevel::Error:
      std::cout << "error";
      break;
    }
    std::cout << "]";

    std::cout << "["
              << Time::format(std::chrono::system_clock::to_time_t(item->time_))
              << "][" << Path::basename(item->file_) << ":" << item->line_
              << "]:" << item->content_;
    if (item->content_.empty() || item->content_.back() != '\n')
      std::cout << std::endl;
  }
}

inline Logger::Logger() { sinks_.push_back(newShared<ConsoleSink>()); }

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

  return print(LoggerLevel::Debug, file, line, std::forward<Targs>(args)...);
}

template <class... Targs>
inline Logger &Logger::printInfo(const char *file, int line, Targs &&... args) {
  if (level_ > LoggerLevel::Info)
    return *this;

  return print(LoggerLevel::Info, file, line, std::forward<Targs>(args)...);
}

template <class... Targs>
inline Logger &Logger::printWarn(const char *file, int line, Targs &&... args) {
  if (level_ > LoggerLevel::Warn)
    return *this;

  return print(LoggerLevel::Warn, file, line, std::forward<Targs>(args)...);
}

template <class... Targs>
inline Logger &Logger::printError(const char *file, int line,
                                  Targs &&... args) {
  if (level_ > LoggerLevel::Error)
    return *this;

  return print(LoggerLevel::Error, file, line, std::forward<Targs>(args)...);
}

template <class... Targs>
inline Logger &Logger::print(LoggerLevel lv, const char *file, int line,
                             Targs &&... args) {
  auto item = newShared<LoggerItem>();
  item->file_ = file;
  item->line_ = line;
  item->lv_ = lv;
  item->content_ = loggerimpl::format(std::forward<Targs>(args)...);

  for (auto sink : sinks_) {
    sink->put(item);
  }
  return *this;
}

MYSPACE_END
