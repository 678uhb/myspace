
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/http/_/uri.hpp"
#include "myspace/strings/sstream.hpp"
#include "myspace/strings/strings.hpp"
MYSPACE_BEGIN

namespace http {

class Response {
public:
  MYSPACE_EXCEPTION_DEFINE(ResponseError, myspace::Exception)

public:
  typedef std::map<std::string, std::string> Header;
  typedef std::string Body;

public:
  void parseHeader(const std::string &);

  void setBody(const std::string &);
  void setBody(std::string &&);

  const Header &header() const;

  const Body &body() const;

  uint32_t status() const;

  std::string statusdesc() const;

  std::string toString() const;

private:
  uint32_t status_;

  std::string statusdesc_;

  Header header_;

  Body body_;
};

inline void Response::parseHeader(const std::string &t_header) {
  std::string line;
  std::stringstream ss(t_header);
  size_t no = 0;
  for (auto &line : Strings::splitBy(t_header, "\r\n")) {
    if (no == 0) {
      auto tokens = Strings::splitOf(line, ' ');
      MYSPACE_THROW_IF_EX(ResponseError, tokens.size() < 2);
      status_ = StringStream(tokens[1]);
      if (tokens.size() >= 3) {
        statusdesc_ = tokens[2];
      }
    } else {
      auto pos = line.find_first_of(":");
      if (pos != line.npos) {
        header_[Strings::stripOf(line.substr(0, pos))] =
            Strings::stripOf(line.substr(pos + 1));
      }
    }
    no++;
  }
}

inline void Response::setBody(const std::string &body) { body_ = body; }
inline void Response::setBody(std::string &&body) { body_.swap(body); }

inline std::string Response::toString() const {
  std::string result;
  result.append("HTTP/1.1 ");
  result.append(StringStream(status_).str());
  result.append(" ");
  result.append(statusdesc_);
  result.append("\r\n");
  for (auto &p : header_) {
    result.append(p.first);
    result.append(": ");
    result.append(p.second);
    result.append("\r\n");
  }
  result.append("\r\n");
  result.append(body_);
  return result;
}

inline const Header &Response::header() const { return header_; }

inline const Body &Response::body() const { return body_; }

} // namespace http

MYSPACE_END
