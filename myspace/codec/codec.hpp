
#pragma once
#include "myspace/logger/logger.hpp"
#include "myspace/memory/memory.hpp"
#include "myspace/myspace_include.h"
#include "myspace/scope/scope.hpp"
MYSPACE_BEGIN

class Codec {
public:
  static string convertGbkToUtf8(const string &src);

  static string convertUtf8ToGbk(const string &src);

  static string encodeBase64(const string &src);

  static string decodeBase64(const string &src);

private:
  static size_t EncodedLength(size_t length);

  static void a3_to_a4(uint8_t *a3, uint8_t *a4);
  static void a4_to_a3(uint8_t *a4, uint8_t *a3);

  constexpr static const char *const base64encode_ =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  constexpr static const char *const base64decode_ =
      "\xFF" // 1
      "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
      "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF" // 32
                                                                         // (33)

      "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"     // 10 (43)
      "\x3E"                                         // 1 / (44)
      "\xFF\xFF\xFF"                                 // 3   (47)
      "\x3F\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D" // 11 + 0 - 9 (58)
      "\xFF\xFF\xFF\xFF\xFF\xFF\xFF"                 // 7 (65)
      "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C"
      "\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19" // 26 A - Z (91)
      "\xFF\xFF\xFF\xFF\xFF\xFF"                             // 6 (97)
      "\x1A\x1B\x1C\x1D\x1E\x1F\x20\x21\x22\x23\x24\x25\x26"
      "\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F\x30\x31\x32\x33"; // 26 a- z (123)
};

#ifdef MYSPACE_LINUX

class Iconv {
public:
  Iconv(const char *from, const char *to);

  Iconv(const string &from, const string &to);

  ~Iconv();

  string convert(const string &src);

private:
  iconv_t iconv_;
};

#endif

#ifdef MYSPACE_WINDOWS

wstring toWideChar(uint32_t codepage, const string &src);

string toMultiByte(uint32_t codepage, const wstring &src);

#endif

inline string Codec::convertGbkToUtf8(const string &src) {
#ifdef MYSPACE_LINUX
  return Iconv("GB2312", "UTF-8").convert(src);
#else
  return toMultiByte(65001, toWideChar(936, src));
#endif
}

inline string Codec::convertUtf8ToGbk(const string &src) {
#ifdef MYSPACE_LINUX
  return Iconv("UTF-8", "GB2312").convert(src);
#else
  return toMultiByte(936, toWideChar(65001, src));
#endif
}

inline string Codec::encodeBase64(const string &src) {
  string result;
  uint8_t a3[3] = {0}, a4[4] = {0};
  size_t i = 0;
  result.reserve(EncodedLength(src.size()));
  for (auto c : src) {
    a3[i++] = (uint8_t)c;
    if (i == 3) {
      i = 0;
      a3_to_a4(a3, a4);
      for (auto j = 0; j < 4; ++j) {
        result.append(1, base64encode_[a4[j]]);
      }
    }
  }
  if (i != 0) {
    for (size_t j = i; j < 3; ++j)
      a3[j] = 0;
    a3_to_a4(a3, a4);
    for (size_t j = 0; j < i + 1; ++j)
      result.append(1, base64encode_[a4[j]]);
    while (i++ < 3)
      result.append(1, '=');
  }
  return move(result);
}

inline string Codec::decodeBase64(const string &src) {
  string result;
  size_t i = 0;
  uint8_t a3[3] = {0}, a4[4] = {0};
  for (auto c : src) {
    if (c == '=') {
      break;
    }
    if (c > 'z') {
      break;
    }
    a4[i] = base64decode_[c];
    // MYSPACE_DEV("c = %s, a4[i] = %s", (int)c, (int)a4[i]);
    if (a4[i] == '\xff') {
      break;
    }
    ++i;
    if (i >= 4) {
      i = 0;
      a4_to_a3(a4, a3);
      for (auto a : a3) {
        result.append(1, a);
      }
    }
  }
  // MYSPACE_DEV("i = ", i);
  if (i == 2) {
    // MYSPACE_DEV(" a4[0] = %s, a4[1] = %s", (size_t)a4[0], (size_t)a4[1]);
    result.append(1, (a4[0] << 2) | (a4[1] >> 4));
  } else if (i == 3) {
    result.append(1, (a4[0] << 2) | (a4[1] >> 4));
    result.append(1, (a4[1] << 4) | (a4[2] >> 2));
  }
  return move(result);
}

inline size_t Codec::EncodedLength(size_t length) {
  return (length + 2 - ((length + 2) % 3)) / 3 * 4;
}

inline void Codec::a3_to_a4(uint8_t *a3, uint8_t *a4) {
  a4[0] = (a3[0] & 0xfc) >> 2;
  a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
  a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
  a4[3] = (a3[2] & 0x3f);
}

inline void Codec::a4_to_a3(uint8_t *a4, uint8_t *a3) {
  a3[0] = ((a4[0] << 2) & 0xfc) | ((a4[1] >> 4) & 0x03);
  a3[1] = ((a4[1] << 4) & 0xf0) | ((a4[2] >> 2) & 0x0f);
  a3[2] = ((a4[2] << 6) & 0xc0) | (a4[3] & 0x3f);
}

#ifdef MYSPACE_WINDOWS

inline wstring toWideChar(uint32_t codepage, const string &src) {
  auto dstlen =
      ::MultiByteToWideChar(codepage, 0, src.c_str(), src.size(), nullptr, 0);
  if (dstlen <= 0)
    return wstring();
  auto dstbuffer = new_unique<wchar_t[]>(dstlen);
  auto n = ::MultiByteToWideChar(codepage, 0, src.c_str(), src.size(),
                                 dstbuffer.get(), dstlen);
  if (n <= 0)
    return wstring();
  return wstring(dstbuffer.get(), min(n, dstlen));
}

inline string toMultiByte(uint32_t codepage, const wstring &src) {
  auto dstlen = ::WideCharToMultiByte(codepage, 0, src.c_str(), src.size(),
                                      nullptr, 0, nullptr, nullptr);
  if (dstlen <= 0)
    return string();
  auto dstbuffer = new_unique<char[]>(dstlen);
  auto n = ::WideCharToMultiByte(codepage, 0, src.c_str(), src.size(),
                                 dstbuffer.get(), dstlen, nullptr, nullptr);
  if (n <= 0)
    return string();
  // MYSPACE_DEV(" to multi bytes result %s",string(dstbuffer.get(), min(n,
  // dstlen)));
  return string(dstbuffer.get(), min(n, dstlen));
}

#endif

#ifdef MYSPACE_LINUX

inline Iconv::Iconv(const char *from, const char *to)
    : iconv_(iconv_open(to, from)) {}

inline Iconv::Iconv(const string &from, const string &to)
    : iconv_(iconv_open(to.c_str(), from.c_str())) {}

inline Iconv::~Iconv() { iconv_close(iconv_); }

inline string Iconv::convert(const string &src) {
  if (src.empty())
    return "";
  char *psrc = (char *)src.c_str();
  size_t srcleft = src.size();
  const size_t dstlen = max(size_t(128), min(size_t(1024), srcleft));
  auto dst = new_unique<char[]>(dstlen);
  // MYSPACE_DEV(" begin srclen = %s, dstlen = %s", srcleft, dstlen);
  string result;
  while (srcleft > 0) {
    char *pdst = dst.get();
    size_t dstleft = dstlen;
    size_t n = iconv(iconv_, &psrc, &srcleft, &pdst, &dstleft);
    if (n == size_t(-1)) {
      //  MYSPACE_DEV(" errno = %s, srcleft = %s, dstleft = %s", errno,
      //  srcleft,dstleft);
      if (errno == EILSEQ || errno == EINVAL) {
        break;
      } else if (errno == E2BIG) {
        result.append(dst.get(), dstlen - dstleft);
      }
    } else {
      // MYSPACE_DEV(" iconv n = %s", n);
      result.append(dst.get(), dstlen - dstleft);
    }
  }
  // MYSPACE_DEV("result = %s, length = %s", result, result.size());
  return move(result);
}

#endif

MYSPACE_END
