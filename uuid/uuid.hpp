
#pragma once

#include "myspace/_/stdafx.hpp"
#if defined(MYSPACE_WINDOWS)
#include <Objbase.h>
#pragma comment(lib, "Ole32.lib")
#endif

MYSPACE_BEGIN

class UUID {
public:
  static std::string gen();
};

inline std::string UUID::gen() {
#if defined(MYSPACE_WINDOWS)
  GUID guid;
  std::string uuid;
  ::CoCreateGuid(&guid);
  {
    std::stringstream ss;
    ss << std::hex << std::setw(sizeof(guid.Data1) << 1) << std::setfill('0')
       << guid.Data1;
    uuid.append(std::move(ss.str()));
    uuid.append(1, '-');
  }
  {
    std::stringstream ss;
    ss << std::hex << std::setw(sizeof(guid.Data2) << 1) << std::setfill('0')
       << guid.Data2;
    uuid.append(std::move(ss.str()));
    uuid.append(1, '-');
  }
  {
    std::stringstream ss;
    ss << std::hex << std::setw(sizeof(guid.Data3) << 1) << std::setfill('0')
       << guid.Data3;
    uuid.append(std::move(ss.str()));
    uuid.append(1, '-');
  }
  for (auto i = 0; i < sizeof(guid.Data4); ++i) {
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)guid.Data4[i];
    uuid.append(std::move(ss.str()));
  }
  return std::move(uuid);
#endif
#if defined(MYSPACE_LINUX)
  uuid_t uid;
  uuid_generate(uid);
  std::stringstream ss;
  for (size_t i = 0; i < sizeof(uid); ++i)
    ss << std::hex << std::setw(2) << std::setfill('0')
       << (int)((char *)&uid)[0];
  return std::move(ss.str());
#endif
}

MYSPACE_END
