
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/net/addr.hpp"
#include "myspace/strings/strings.hpp"

MYSPACE_BEGIN

namespace http {
class Uri {
  MYSPACE_EXCEPTION_DEFINE(UriError, myspace::Exception)
public:
  Uri(const std::string &uri) noexcept(false);
  const std::string &domain() const;
  const std::string &path() const;
  const std::string &suburi() const;
  uint16_t port() const;
  std::string &domain();
  std::string &path();
  std::string &suburi();
  uint16_t &port();

  std::string debug() const;

private:
  uint16_t port_ = 80;
  std::string domain_;
  std::string suburi_;
  std::string path_;
  std::map<std::string, std::string> params_;
};

inline const std::string &Uri::domain() const { return domain_; }
inline const std::string &Uri::path() const { return path_; }
inline const std::string &Uri::suburi() const { return suburi_; }
inline uint16_t Uri::port() const { return port_; }
inline std::string &Uri::domain() { return domain_; }
inline std::string &Uri::path() { return path_; }
inline std::string &Uri::suburi() { return suburi_; }
inline uint16_t &Uri::port() { return port_; }

inline Uri::Uri(const std::string &t_uri) noexcept(false) {
  auto uri = Strings::tolower(t_uri);
  if (Strings::startWith(uri, "http:")) {
    uri.erase(0, 5);
  }
  uri = Strings::stripOf(uri, '/');
  auto firstsplit = uri.find_first_of("/?");
  {
    auto domain_port = uri.substr(0, firstsplit);
    auto pos = domain_port.find(':');
    if (pos == uri.npos) {
      domain_.swap(domain_port);
    } else {
      domain_ = domain_port.substr(0, pos);
      port_ = StringStream(domain_port.substr(pos + 1));
    }
  }

  auto question_mark = uri.find("?");
  if (question_mark != uri.npos) {
    if (firstsplit < question_mark) {
      path_ = '/' + Strings::stripOf(uri.substr(firstsplit + 1,
                                                question_mark - firstsplit - 1),
                                     '/');
      suburi_ = '/' + Strings::stripOf(uri.substr(firstsplit + 1), '/');
    }
    auto params = uri.substr(question_mark + 1);
    auto tokens = Strings::splitOf(params, '&');
    for (auto &token : tokens) {
      auto subtokens = Strings::splitOf(token, '=');
      if (subtokens.size() == 1) {
        params_[subtokens[0]];
      } else if (subtokens.size() > 1) {
        params_[subtokens[0]] = subtokens[1];
      }
    }
  }
  if (path_.empty())
    path_ = '/';
  if (suburi_.empty())
    suburi_ = '/';
}
inline std::string Uri::debug() const {
  std::string result = "addr: " + domain_ + ":" + StringStream(port_).str() +
                       "\n" + "suburi: " + suburi_ + "\n";

  if (!params_.empty()) {
    result += "params:\n";
    for (auto &p : params_) {
      result += "\t" + p.first;
      if (!p.second.empty()) {
        result += ": " + p.second;
      }
      result += "\n";
    }
  }

  return result;
}
} // namespace http

MYSPACE_END
