
#pragma once
#include "myspace/memory/memory.hpp"
#include "myspace/myspace_include.h"

MYSPACE_BEGIN

class Codec {
public:
  static string convertGbkToUtf8(const string &src);

  static string convertUtf8ToGbk(const string &src);

  static string encodeBase64(const string &src);

private:
  static size_t EncodedLength(size_t length);

  static void a3_to_a4(unsigned char *a3, unsigned char *a4);

  static constexpr const char *const kBase64Alphabet_ =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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

string toMultiChar(uint32_t codepage, const wstring &src);

#endif

inline string Codec::convertGbkToUtf8(const string &src) {
#ifdef MYSPACE_LINUX
  return Iconv("GB2312", "UTF-8").convert(src);
#else
  return toMultiChar(65001, toWideChar(936, src));
#endif
}

inline string Codec::convertUtf8ToGbk(const string &src) {
#ifdef MYSPACE_LINUX
  return Iconv("UTF-8", "GB2312").convert(src);
#else
  return toMultiChar(936, toWideChar(65001, src));
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
        result.append(1, kBase64Alphabet_[a4[j]]);
      }
    }
  }
  if (i != 0) {
    for (size_t j = i; j < 3; ++j)
      a3[j] = 0;
    a3_to_a4(a3, a4);
    for (size_t j = 0; j < i + 1; ++j)
      result.append(1, kBase64Alphabet_[a4[j]]);
    while (i++ < 3)
      result.append(1, '=');
  }
  return move(result);
}

inline size_t Codec::EncodedLength(size_t length) {
  return (length + 2 - ((length + 2) % 3)) / 3 * 4;
}

inline void Codec::a3_to_a4(unsigned char *a3, unsigned char *a4) {
  a4[0] = (a3[0] & 0xfc) >> 2;
  a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
  a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
  a4[3] = (a3[2] & 0x3f);
}

#ifdef MYSPACE_WINDOWS

inline wstring toWideChar(uint32_t codepage, const string &src) {
  auto dstlen = ::MultiByteToWideChar(codepage, 0, src.c_str(), -1, nullptr, 0);
  if (dstlen <= 0)
    return wstring();
  auto dstbuffer = new_unique<wchar_t[]>(dstlen);
  auto n = ::MultiByteToWideChar(codepage, 0, src.c_str(), -1, dstbuffer.get(),
                                 dstlen);
  if (n <= 0)
    return wstring();
  return wstring(dstbuffer.get(), min(n, dstlen));
}

inline string toMultiChar(uint32_t codepage, const wstring &src) {
  auto dstlen = ::WideCharToMultiByte(codepage, 0, src.c_str(), src.size(),
                                      nullptr, 0, nullptr, nullptr);
  if (dstlen <= 0)
    return string();
  auto dstbuffer = new_unique<char[]>(dstlen);
  auto n = ::WideCharToMultiByte(codepage, 0, src.c_str(), src.size(),
                                 dstbuffer.get(), dstlen, nullptr, nullptr);
  if (n <= 0)
    return string();
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
  const size_t srclen = src.size();
  char *psrc = (char *)src.c_str();
  char **ppsrc = &psrc;
  size_t srcleft = sizeof(src.size());
  size_t *psrcleft = &srcleft;
  const size_t dstlen = max(srclen, size_t(8));
  auto dst = new_unique<char[]>(dstlen);
  char *pdst = dst.get();
  char **ppdst = &pdst;
  size_t dstleft = dstlen;
  size_t *pdstleft = &dstleft;
  string result;
  for (;;) {
    auto n = iconv(iconv_, ppsrc, psrcleft, ppdst, pdstleft);
    if (n < 0) {
      if (errno == EILSEQ || errno == EINVAL) {
        *ppsrc++;
        *psrcleft--;
        continue;
      } else if (errno == E2BIG) {
        result.append(dst.get(), dstlen - dstleft);
        dstleft = dstlen;
        pdst = dst.get();
        continue;
      } else {
        break;
      }
    } else {
      result.append(dst.get(), dstlen - dstleft);

      break;
    }
  }
  return move(result);
}

#endif

MYSPACE_END
