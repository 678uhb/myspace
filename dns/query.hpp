
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/dns/dns/dns.hpp"
#include "myspace/net/addr.hpp"
#include "myspace/net/tcp/socket.hpp"
#include "myspace/net/udp/socket.hpp"
#include "myspace/time/time.hpp"
MYSPACE_BEGIN

namespace dns {
class Resolver {
public:
  MYSPACE_EXCEPTION_DEFINE(ResolverError, myspace::Exception)
  MYSPACE_EXCEPTION_DEFINE(NotFound, ResolverError)
  MYSPACE_EXCEPTION_DEFINE(TimeOut, ResolverError)
  MYSPACE_EXCEPTION_DEFINE(NoDnsServer, ResolverError)

public:
  std::deque<Addr> query(const std::string &domain_name,
                         std::chrono::high_resolution_clock::duration timeout =
                             std::chrono::seconds(3)) throw(NotFound, TimeOut);

  std::deque<Addr> query(const Addr &server, const std::string &domain_name,
                         std::chrono::high_resolution_clock::duration timeout =
                             std::chrono::seconds(3)) throw(NotFound, TimeOut);
};

inline std::deque<Addr>
Resolver::query(const std::string &domain_name,
                std::chrono::high_resolution_clock::duration
                    timeout) throw(Resolver::NotFound, Resolver::TimeOut) {
  try {
    auto serveraddr = dnsimpl::systemDnsList();
    MYSPACE_THROW_IF(serveraddr.empty());
    return query(serveraddr.front(), domain_name, timeout);

  } catch (...) {
    MYSPACE_THROW_EX(Resolver::NotFound);
  }
  return std::deque<Addr>{}; // not reached;
}

inline std::deque<Addr>
Resolver::query(const Addr &server, const std::string &domain_name,
                std::chrono::high_resolution_clock::duration
                    timeout) throw(Resolver::NotFound, Resolver::TimeOut) {
  try {
    std::deque<Addr> result{};
    dnsimpl::Message req;
    req.header_.id_ = dnsimpl::getId();
    req.header_.flags_ = 0x0100;
    {
      dnsimpl::Question q;
      q.qname_ = domain_name;
      q.qtype_ = dnsimpl::TYPE::A;
      q.qclass_ = dnsimpl::QCLASS::IN_;
      req.question_.emplace_back(q);
    }

    auto resp = dnsimpl::Compressor<SocketStream<udp::Socket>>{server, timeout}
                    .compress(req)
                    .extract();

    MYSPACE_DEV(dnsimpl::dump(resp));
    for (auto &x : resp.answer_) {
      if (x.type_ == dnsimpl::TYPE::A) {
        sockaddr_in addr;
        addr.sin_port = Codec::hton((uint16_t)80);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = *(uint32_t *)x.rdata_.c_str();
        result.emplace_back(addr);
      }
    }
    return result;

  } catch (...) {
    MYSPACE_THROW_EX(Resolver::NotFound);
  }
  return std::deque<Addr>{}; // not reached;
}

} // namespace dns

MYSPACE_END
