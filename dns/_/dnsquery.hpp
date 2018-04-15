

#pragma once

#include "myspace/_/stdafx.hpp"

#if defined(MYSPACE_WINDOWS)
#include "myspace/error/error.hpp"
#include "myspace/net/addr.hpp"
MYSPACE_BEGIN

namespace dns {

struct Result {
  std::string a_;
};

inline Result dnsQuery(const std::string &name, uint16_t type = DNS_TYPE_A,
                       uint32_t flags = DNS_QUERY_STANDARD |
                                        DNS_QUERY_DONT_RESET_TTL_VALUES) {
  Result ret;
  PDNS_RECORD pdns_record = nullptr;
  DNS_STATUS status =
      ::DnsQuery(name.c_str(), type, flags, nullptr, &pdns_record, nullptr);
  if (pdns_record) {
    MYSPACE_DEFER(if (pdns_record)::DnsRecordListFree(pdns_record,
                                                      DnsFreeRecordListDeep););
    if (status) {
      MYSPACE_DEV(Error::strerror(status));
    } else {
      if (type == DNS_TYPE_A && pdns_record->wType == DNS_TYPE_A) {
        if (pdns_record->Flags.DW & DNSREC_ANSWER) {
          in_addr addr;
          addr.S_un.S_addr = pdns_record->Data.A.IpAddress;
          ret.a_ = Addr::inetNtop(addr, AF_INET);
        }
      }
    }
  }
  return ret;
}
} // namespace dns

MYSPACE_END

#endif
