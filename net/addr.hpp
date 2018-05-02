
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/strings/sstream.hpp"
#include "myspace/strings/strings.hpp"

MYSPACE_BEGIN

class Addr {
public:
  static Addr local(int sock);

  static Addr peer(int sock);

  static std::string inetNtop(const in_addr &addr, int32_t family = AF_INET);

  static in_addr inetPton(const std::string &ip, int32_t family = AF_INET);

  Addr();

  Addr(const Addr &) = default;

  Addr(Addr &&) = default;

  Addr(const std::string &ip, uint16_t port);

  Addr(sockaddr_in addr);

  Addr(const std::string &addr);

  Addr &operator=(const Addr &) = default;

  std::string toString() const;

  Addr &setPort(uint16_t port);

  Addr &setFamily(int32_t family);

  const sockaddr_in &addr() const;

private:
  sockaddr_in addr_ = { 0 };
};

inline Addr::Addr() { addr_.sin_family = AF_INET; }

inline Addr::Addr(sockaddr_in addr) : addr_(addr) {}

inline Addr::Addr(const std::string &ip, uint16_t port) : Addr() {
  addr_.sin_port = htons(port);
  addr_.sin_addr = Addr::inetPton(ip, AF_INET);
}

inline Addr &Addr::setPort(uint16_t port) {
  addr_.sin_port = htons(port);
  return *this;
}
inline Addr &Addr::setFamily(int32_t family) {
  addr_.sin_family = family;
  return *this;
}

inline Addr::Addr(const std::string &addr) : Addr() {
  auto tokens = Strings::splitOf(addr, ':');
  if (tokens.size() >= 2) {
    uint16_t port = StringStream(tokens[1]);
    addr_.sin_port = htons(port);
    addr_.sin_family = AF_INET;
    addr_.sin_addr = Addr::inetPton(tokens[0], AF_INET);
  }
}

inline Addr Addr::local(int sock) {
  sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  getsockname(sock, (sockaddr *)&addr, &addrlen);
  return Addr(addr);
}
inline Addr Addr::peer(int sock) {
  sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  getpeername(sock, (sockaddr *)&addr, &addrlen);
  return Addr(addr);
}

inline const sockaddr_in &Addr::addr() const { return addr_; }

inline std::string Addr::inetNtop(const in_addr &addr, int32_t family) {
  static constexpr size_t bufflen = 128;
  auto buff = newUnique<char[]>(bufflen);
#if defined(MYSPACE_WINDOWS)
  return ::inet_ntop(family, (PVOID) & addr, buff.get(), bufflen);
#else
  return ::inet_ntop(family, (const void *)&addr, buff.get(), bufflen);
#endif
}

inline in_addr Addr::inetPton(const std::string &ip, int32_t family) {
  in_addr addr{};
  ::inet_pton(family, ip.c_str(), &addr);
  return addr;
}

inline std::string Addr::toString() const {
  std::stringstream ss;
  ss << Addr::inetNtop(addr_.sin_addr, addr_.sin_family) << ":"
     << ntohs(addr_.sin_port);
  return ss.str();
}

inline bool operator==(const sockaddr_in &l, const sockaddr_in &r) {
  return l.sin_family == r.sin_family && l.sin_port == r.sin_port &&
         l.sin_addr.s_addr == r.sin_addr.s_addr;
}

MYSPACE_END
