
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/http/_/structure.hpp"
#include "myspace/http/_/uri.hpp"
MYSPACE_BEGIN

namespace http {

class Request {
public:
  Request(const http::Uri &uri, const http::Header &header = {},
          const http::Body &body = {});

  std::string toString(http::Method method) const;

  Uri &uri();

  Header &header();

  Body &body();

  const Uri &uri() const;

  const Header &header() const;

  const Body &body() const;

private:
  http::Uri uri_;
  http::Header header_;
  http::Body body_;
};

inline std::string Request::toString(http::Method method) const {
  std::string result;
  switch (method) {
  case Method::GET:
    result.append("GET");
    break;
  case Method::POST:
    result.append("POST");
    break;
  case Method::PUT:
    result.append("PUT");
    break;
  case Method::DELT:
    result.append("DELETE");
    break;
  }
  result.append(" ");
  result.append(uri_.suburi());
  result.append(" HTTP/1.1\r\n");

  for (auto &p : header_) {
    result.append(p.first);
    result.append(": ");
    result.append(p.second);
    result.append("\r\n");
  }
  if (!body_.empty() && (header_.find("Content-length") == header_.end())) {
    result.append("Content-length: ");
    result.append(StringStream(body_.size()).str());
    result.append("\r\n");
  }
  result.append("\r\n");
  if (!body_.empty()) {
    result.append(body_);
  }
  return result;
}

inline http::Uri &Request::uri() { return uri_; }
inline http::Header &Request::header() { return header_; }
inline http::Body &Request::body() { return body_; }

inline const http::Uri &Request::uri() const { return uri_; }
inline const http::Header &Request::header() const { return header_; }
inline const http::Body &Request::body() const { return body_; }

inline Request::Request(const http::Uri &uri, const http::Header &header,
                        const http::Body &body)
    : uri_(uri), body_(body) {
  for (auto &p : header) {
    header_[p.first] = p.second;
  }
}

} // namespace http

MYSPACE_END
