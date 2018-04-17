
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/codec/codec.hpp"
#include "myspace/error/error.hpp"
#include "myspace/net/addr.hpp"
#include "myspace/net/netstream.hpp"
#include "myspace/net/socketstream.hpp"
#include "myspace/strings/strings.hpp"
MYSPACE_BEGIN

namespace dns {

namespace dnsimpl {

struct Message;
struct Header;
struct Question;
struct ResourceRecord;
inline std::deque<Addr> systemDnsList();
inline uint16_t getId();
inline std::string dump(const Message &m);
template <class StreamType> class Compressor;
// The header contains the following fields:

//     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//   |                      ID                       |
//   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//   |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
//   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//   |                    QDCOUNT                    |
//   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//   |                    ANCOUNT                    |
//   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//   |                    NSCOUNT                    |
//   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//   |                    ARCOUNT                    |
//   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

// more detail, see rfc1034, rfc1035
// https://www.ietf.org/rfc/rfc1034.txt
// https://www.ietf.org/rfc/rfc1035.txt

struct Header {
  uint16_t id_ = 0;
  uint16_t flags_ = 0;
  // when pack will change these value according to array length
  mutable uint16_t qdcount_ = 0;
  mutable uint16_t ancount_ = 0;
  mutable uint16_t nscount_ = 0;
  mutable uint16_t arcount_ = 0;
};

struct TYPE {
  constexpr static uint16_t A = 1;     //  a host address
  constexpr static uint16_t NS = 2;    // an authoritative name server
  constexpr static uint16_t MD = 3;    // a mail destination (Obsolete - use MX)
  constexpr static uint16_t MF = 4;    //  a mail forwarder (Obsolete - use MX)
  constexpr static uint16_t CNAME = 5; //  the canonical name for an alias
  constexpr static uint16_t SOA = 6;   // marks the start of a zone of authority
  constexpr static uint16_t MB = 7;    // a mailbox domain name (EXPERIMENTAL)
  constexpr static uint16_t MG = 8;    // a mail group member (EXPERIMENTAL)
  constexpr static uint16_t MR = 9; // a mail rename domain name (EXPERIMENTAL)
  constexpr static uint16_t NUL = 10;   // a null RR (EXPERIMENTAL)
  constexpr static uint16_t WKS = 11;   // a well known service description
  constexpr static uint16_t PTR = 12;   // a domain name pointer
  constexpr static uint16_t HINFO = 13; // host information
  constexpr static uint16_t MINFO = 14; // mailbox or mail list information
  constexpr static uint16_t MX = 15;    // mail exchange
  constexpr static uint16_t TXT = 16;   // text strings
};

struct QTYPE : public TYPE {
  constexpr static uint16_t AXFR = 252;  // a transfer of an entire zone
  constexpr static uint16_t MAILB = 253; // mailbox-related records(MB,MG or MR)
  constexpr static uint16_t MAILA = 254; // mail agent RRs (Obsolete - see MX)
  constexpr static uint16_t ALL = 255;   // all records
};

struct CLASS {
  constexpr static uint16_t IN_ = 1; // the Internet
  constexpr static uint16_t CS = 2; // the CSNET class (Obsolete - used only for
                                    // examples in some obsolete RFCs)
  constexpr static uint16_t CH = 3; // the CHAOS class
  constexpr static uint16_t HS = 4; // Hesiod [Dyer 87]
};

struct QCLASS : public CLASS {
  constexpr static uint16_t ANY = 255; // any class
};

struct Question {
  // a domain-name represented as a sequence of labels, where each label
  // consists of a length octet followed by that number of octets. The domain
  // name terminates with the zero length octet for the null label of the root.
  // Note that this field may be an odd number of octets; no padding is used.
  std::string qname_;

  // a two octet code which specifies the type of the query. The values for this
  // field include all codes valid for a TYPE field, together with some more
  // general codes which can match more than one TYPE of RR.
  uint16_t qtype_ = 0;

  // a two octet code that specifies the class of the query. For example, the
  // QCLASS field is IN for the Internet.
  uint16_t qclass_ = 0;
};

struct ResourceRecord {
  // A domain-name to which this resource record pertains.
  std::string name_;

  // two octets containing one of the RR type codes. This field specifies the
  // meaning of the data in the RDATA field.
  uint16_t type_;

  // two octets which specify the class of the data in the RDATA field.
  uint16_t class_;

  // a 32 bit unsigned integer that specifies the time interval (in
  // std::chrono::seconds) that the resource record may be cached before it
  // should be discarded. Zero values are interpreted to mean that the RR can
  // only be used for the transaction in progress, and should not be cached.
  uint32_t ttl_;

  // an unsigned 16 bit integer that specifies the length in octets of the RDATA
  // field.
  uint16_t rdlength_;

  // a variable length std::string of octets that describes the resource. The
  // format of this information varies according to the TYPE and CLASS of the
  // resource record. For example, the if the TYPE is A and the CLASS is IN, the
  // RDATA field is a 4 octet ARPA Internet address.
  std::string rdata_;
};

struct Message {
  Header header_;
  std::deque<Question> question_;
  std::deque<ResourceRecord> answer_;
  std::deque<ResourceRecord> authority_;
  std::deque<ResourceRecord> additional_;
};

// The compression scheme allows a domain name in a message to be represented as
// either:
// a sequence of labels ending in a zero octet
// a pointer
// a sequence of labels ending with a pointer
template <class StreamType = SocketStream<udp::Socket>> class Compressor {
public:
  MYSPACE_EXCEPTION_DEFINE(CompressorError, myspace::Exception)
  MYSPACE_EXCEPTION_DEFINE(CompressError, CompressorError)
  MYSPACE_EXCEPTION_DEFINE(ExtractError, CompressorError)
public:
  Compressor(const Addr &addr) : stream_(addr) {}

  Compressor(const Addr &addr,
             std::chrono::high_resolution_clock::duration timeout)
      : stream_(addr, timeout) {}

  Compressor &compress(const Message &m) throw(CompressError) {
    try {
      stream_.hold(m.header_.id_);
      stream_.hold(m.header_.flags_);
      stream_.hold(m.header_.qdcount_ = (uint16_t)m.question_.size());
      stream_.hold(m.header_.ancount_ = (uint16_t)m.answer_.size());
      stream_.hold(m.header_.nscount_ = (uint16_t)m.authority_.size());
      stream_.hold(m.header_.arcount_ = (uint16_t)m.additional_.size());
      for (const auto &x : m.question_) {
        stream_.hold(compress(x.qname_, (uint16_t)stream_.holdSize()));
        stream_.hold(x.qtype_);
        stream_.hold(x.qclass_);
      }
      for (const auto &x : m.answer_) {
        stream_.hold(compress(x.name_, (uint16_t)stream_.holdSize()));
        stream_.hold(x.type_);
        stream_.hold(x.class_);
        stream_.hold(x.ttl_);
        stream_.hold(x.rdlength_);
        stream_.hold(x.rdata_);
      }
      for (const auto &x : m.authority_) {
        stream_.hold(compress(x.name_, (uint16_t)stream_.holdSize()));
        stream_.hold(x.type_);
        stream_.hold(x.class_);
        stream_.hold(x.ttl_);
        stream_.hold(x.rdlength_);
        stream_.hold(x.rdata_);
      }
      for (const auto &x : m.additional_) {
        stream_.hold(compress(x.name_, (uint16_t)stream_.holdSize()));
        stream_.hold(x.type_);
        stream_.hold(x.class_);
        stream_.hold(x.ttl_);
        stream_.hold(x.rdlength_);
        stream_.hold(x.rdata_);
      }
      stream_.flush();
    } catch (...) {
      MYSPACE_THROW_EX(CompressError);
    }
    return *this;
  }

  Message extract() throw(ExtractError) {
    try {

      Message dst;
      compressed_ = stream_.template peek<std::string>(12);
      stream_ >> dst.header_.id_;
      stream_ >> dst.header_.flags_;
      stream_ >> dst.header_.qdcount_;
      stream_ >> dst.header_.ancount_;
      stream_ >> dst.header_.nscount_;
      stream_ >> dst.header_.arcount_;
      for (size_t i = 0; i < dst.header_.qdcount_; ++i) {
        Question q;
        q.qname_ = unpackDomainName();
        compressed_.append(stream_.template peek<std::string>(4));
        stream_ >> q.qtype_;
        stream_ >> q.qclass_;
        dst.question_.emplace_back(q);
      }
      for (size_t i = 0; i < dst.header_.ancount_; ++i) {
        ResourceRecord r;
        r.name_ = unpackDomainName();
        compressed_.append(stream_.template peek<std::string>(10));
        stream_ >> r.type_;
        stream_ >> r.class_;
        stream_ >> r.ttl_;
        stream_ >> r.rdlength_;
        compressed_.append(stream_.template peek<std::string>(r.rdlength_));
        r.rdata_ = stream_.template recv<std::string>(r.rdlength_);
        dst.answer_.emplace_back(r);
      }
      for (size_t i = 0; i < dst.header_.nscount_; ++i) {
        ResourceRecord r;
        r.name_ = unpackDomainName();
        compressed_.append(stream_.template peek<std::string>(10));
        stream_ >> r.type_;
        stream_ >> r.class_;
        stream_ >> r.ttl_;
        stream_ >> r.rdlength_;
        compressed_.append(stream_.template peek<std::string>(r.rdlength_));
        r.rdata_ = stream_.template recv<std::string>(r.rdlength_);
        dst.authority_.emplace_back(r);
      }
      for (size_t i = 0; i < dst.header_.arcount_; ++i) {
        ResourceRecord r;
        r.name_ = unpackDomainName();
        compressed_.append(stream_.template peek<std::string>(10));
        stream_ >> r.type_;
        stream_ >> r.class_;
        stream_ >> r.ttl_;
        stream_ >> r.rdlength_;
        compressed_.append(stream_.template peek<std::string>(r.rdlength_));
        r.rdata_ = stream_.template recv<std::string>(r.rdlength_);
        dst.additional_.emplace_back(r);
      }
      return dst;
    } catch (...) {
      MYSPACE_THROW_EX(ExtractError);
    }
    return Message{}; // not reached
  }

  void clear() {
    dict_.clear();
    compressed_.clear();
  }

private:
  std::string compress(const std::string &name, uint16_t offset) {
    std::string compressed;
    auto labels = Strings::splitOf(name, '.');
    MYSPACE_THROW_IF_EX(CompressorError, labels.empty());
    for (auto &label : labels) {
      MYSPACE_THROW_IF_EX(CompressorError, label.empty() || label.size() > 63);
    }
    bool has_pointer = false;
    while (!labels.empty()) {
      auto itr = dict_.find(labels);
      if (itr != dict_.end()) {
        compressed.append(makePointer(itr->second));
        has_pointer = true;
        break;
      } else {
        auto &label = labels.front();
        compressed.append(1, (uint8_t)label.size());
        compressed.append(label);
        dict_[labels] = offset;
        offset += (uint16_t)(label.size() + 1);
      }
      labels.pop_front();
    }
    if (!has_pointer) {
      compressed.append(1, '\0');
    }
    return compressed;
  }

  void seekDomainName(uint16_t pointer, std::string &dst) {
    MYSPACE_THROW_IF(!(pointer & 0xc000));
    pointer &= ~0xc000;
    for (size_t max_loop = 1000; max_loop; --max_loop) {
      MYSPACE_THROW_IF(pointer >= compressed_.size());
      auto c = compressed_[pointer];
      if (0xc0 & c) {
        MYSPACE_THROW_IF((size_t)pointer + 2 > compressed_.size());
        pointer = Codec::ntoh(*(uint16_t *)(compressed_.c_str() + pointer));
        pointer &= ~0xc000;
      } else {
        size_t len = c;
        ++pointer;
        MYSPACE_THROW_IF(len > 63);
        if (len == 0)
          break;
        if (!dst.empty())
          dst.append(1, '.');
        MYSPACE_THROW_IF(pointer + len > (uint16_t)compressed_.size());
        dst.append(compressed_.substr(pointer, len));
        pointer += (uint16_t)len;
      }
    }
  }

  std::string unpackDomainName() {
    std::string name;
    for (;;) {
      auto c = stream_.template peek<uint8_t>();
      if (c & 0xc0) {
        compressed_.append(stream_.template peek<std::string>(2));
        seekDomainName(stream_.template recv<uint16_t>(), name);
        break;
      } else {
        compressed_.append(1, c = stream_.template recv<uint8_t>());
        if (c == 0) {
          break;
        }
        MYSPACE_THROW_IF_EX(ExtractError, c > 63);
        if (!name.empty())
          name.append(1, '.');
        auto label = stream_.template recv<std::string>(c);
        compressed_.append(label);
        name.append(label);
      }
    }
    return name;
  }

  std::string makePointer(uint16_t offset) const {
    MYSPACE_THROW_IF_EX(CompressorError, offset < 12);
    MYSPACE_THROW_IF_EX(CompressorError, offset & 0xc000);
    std::string result;
    offset |= 0xc000;
    offset = Codec::hton(offset);
    result.append((char *)&offset, 2);
    return result;
  }

  bool use_timeout_ = false;
  std::chrono::high_resolution_clock::duration timeout_;
  Addr serveraddr_;

  // labels, and its offset
  std::map<std::deque<std::string>, uint16_t> dict_;

  // extract dict
  std::string compressed_;

  StreamType stream_;
};

inline std::deque<Addr> systemDnsList() {
  std::deque<Addr> result;
#if defined(MYSPACE_LINUX)
  std::ifstream ifs("/etc/resolv.conf", std::ios::in | std::ios::binary);
  std::string line;
  while (getline(ifs, line)) {
    auto tokens = Strings::splitOf(line, " \t");
    std::string ip;
    if (tokens.size() == 2 && Strings::tolower(tokens[0]) == "nameserver") {
      ip = Strings::stripOf(tokens[1]);
    }
    if (!ip.empty()) {
      result.emplace_back(ip, 53);
    }
  }
#endif
#if defined(MYSPACE_WINDOWS)
  try {
    ULONG outbuflen = sizeof(FIXED_INFO);
    auto fi = (FIXED_INFO *)malloc(outbuflen);
    MYSPACE_THROW_IF(!fi);
    MYSPACE_DEFER(if (fi) {
      free(fi);
      fi = nullptr;
    });
    auto ret = ::GetNetworkParams(fi, &outbuflen);
    if (ret == ERROR_BUFFER_OVERFLOW) {
      free(fi);
      fi = (FIXED_INFO *)malloc(outbuflen);
      MYSPACE_THROW_IF(!fi);
    }
    ret = ::GetNetworkParams(fi, &outbuflen);
    if (ret == ERROR_SUCCESS) {
      std::string ip = Strings::stripOf(fi->DnsServerList.IpAddress.String);
      result.emplace_back(ip, 53);
      auto pIPAddr = fi->DnsServerList.Next;
      while (pIPAddr) {
        ip = Strings::stripOf(pIPAddr->IpAddress.String);
        result.emplace_back(ip, 53);
        pIPAddr = pIPAddr->Next;
      }
    } else {
      MYSPACE_DEV(Error::strerror(ret));
    }
  } catch (...) {
    MYSPACE_DEV(Exception::dump());
  }
#endif
  return result;
}

inline std::string dump(const Message &m) {
  try {
    std::stringstream ss;
    ss << "header" << std::endl;
    ss << "header.id_: " << m.header_.id_ << std::endl;
    ss << "header.flags: " << std::hex << m.header_.id_ << std::dec
       << std::endl;
    ss << "header.qdcount_: " << m.header_.qdcount_ << std::endl;
    ss << "header.ancount_: " << m.header_.ancount_ << std::endl;
    ss << "header.nscount_: " << m.header_.nscount_ << std::endl;
    ss << "header.arcount_: " << m.header_.arcount_ << std::endl;
    for (size_t i = 0; i < m.question_.size(); ++i) {
      auto &x = m.question_[i];
      ss << "question." << i << std::endl;
      ss << "question.qname_: " << x.qname_ << std::endl;
      ss << "question.qtype_: " << x.qtype_ << std::endl;
      ss << "question.qclass_: " << x.qclass_ << std::endl;
    }
    for (size_t i = 0; i < m.answer_.size(); ++i) {
      auto &x = m.answer_[i];
      ss << "answer." << i << std::endl;
      ss << "answer.name_: " << x.name_ << std::endl;
      ss << "answer.type_: " << x.type_ << std::endl;
      ss << "answer.class_: " << x.class_ << std::endl;
      ss << "answer.ttl_: " << x.ttl_ << std::endl;
      ss << "answer.rdlength_: " << x.rdlength_ << std::endl;
      ss << "answer.rdata_: " << x.rdata_ << std::endl;
    }
    for (size_t i = 0; i < m.authority_.size(); ++i) {
      auto &x = m.authority_[i];
      ss << "authority." << i << std::endl;
      ss << "authority.name_: " << x.name_ << std::endl;
      ss << "authority.type_: " << x.type_ << std::endl;
      ss << "authority.class_: " << x.class_ << std::endl;
      ss << "authority.ttl_: " << x.ttl_ << std::endl;
      ss << "authority.rdlength_: " << x.rdlength_ << std::endl;
      ss << "authority.rdata_: " << x.rdata_ << std::endl;
    }
    for (size_t i = 0; i < m.additional_.size(); ++i) {
      auto &x = m.additional_[i];
      ss << "additional." << i << std::endl;
      ss << "additional.name_: " << x.name_ << std::endl;
      ss << "additional.type_: " << x.type_ << std::endl;
      ss << "additional.class_: " << x.class_ << std::endl;
      ss << "additional.ttl_: " << x.ttl_ << std::endl;
      ss << "additional.rdlength_: " << x.rdlength_ << std::endl;
      ss << "additional.rdata_: " << x.rdata_ << std::endl;
    }
    return ss.str();
  } catch (...) {
    MYSPACE_DEV_EXCEPTION();
  }
  return "";
}

inline uint16_t getId() {
  static std::atomic<uint16_t> a;
  ++a;
  if (a == 0)
    ++a;
  return a;
}

} // namespace dnsimpl
} // namespace dns
MYSPACE_END
