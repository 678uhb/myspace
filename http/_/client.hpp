
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/dns/query.hpp"
#include "myspace/http/_/request.hpp"
#include "myspace/http/_/response.hpp"
#include "myspace/http/_/uri.hpp"
#include "myspace/net/socketstream.hpp"
#include "myspace/strings/strings.hpp"

MYSPACE_BEGIN

namespace http {
class Client {
  MYSPACE_EXCEPTION_DEFINE(ClientError, myspace::Exception)
public:
  Response get(const Request &req,
               std::chrono::high_resolution_clock::duration timeout =
                   std::chrono::seconds(30)) noexcept(false);
  Response post(const Request &req,
                std::chrono::high_resolution_clock::duration timeout =
                    std::chrono::seconds(30)) noexcept(false);
  Response put(const Request &req,
               std::chrono::high_resolution_clock::duration timeout =
                   std::chrono::seconds(30)) noexcept(false);
  Response delt(const Request &req,
                std::chrono::high_resolution_clock::duration timeout =
                    std::chrono::seconds(30)) noexcept(false);

private:
  Response httpMethod(Method method, const Request &req,
                      std::chrono::high_resolution_clock::duration timeout);
};

inline Response Client::get(
    const Request &req,
    std::chrono::high_resolution_clock::duration timeout) noexcept(false) {
  try {
    return httpMethod(Method::GET, req, timeout);
  }
  catch (...) {
    MYSPACE_THROW_EX(ClientError);
  }
  return Response{}; // not reached
}

inline Response
Client::httpMethod(Method method, const Request &t_req,
                   std::chrono::high_resolution_clock::duration timeout) {
  auto req = t_req;
  auto domain = Strings::tolower(req.uri().domain());
  if (Strings::startWith(domain, "www.")) {
    domain = domain.substr(4);
  }
  auto addrs = dns::Resolver().query(domain);
  MYSPACE_THROW_IF(addrs.empty());
  SocketStream<tcp::Socket> ss{ addrs.front(), timeout };

  {
    req.header()["host"] = req.uri().domain();
    auto str = req.toString(method);
    MYSPACE_DEV(str);
    ss.send(str);
  }

  auto header = ss.recvUntil("\r\n\r\n");
  MYSPACE_DEV(header);
  Response resp;
  resp.parseHeader(header);

  auto itr = resp.header().find("Content-Length");
  if (itr != resp.header().end()) {
    size_t bodylen = StringStream(itr->second);
    auto body = ss.recv<std::string>(bodylen);
    resp.setBody(body);
    MYSPACE_DEV(body);
  }
  MYSPACE_DEV(resp.toString());
  // body
  return resp;
}

} // namespace http

MYSPACE_END
