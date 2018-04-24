
#include "myspace/_/stdafx.hpp"
#include "myspace/wav/wav.hpp"

MYSPACE_BEGIN
namespace wav {
class Tool {
public:
  static uint8_t combine(uint8_t pcm1, uint8_t pcm2);

  static int16_t combine(int16_t pcm1, int16_t pcm2);

  template <class X> static X combine(const X &pcm1, const X &pcm2);

  static std::string combine_16_samplebits(const std::string &pcm1,
                                           const std::string &pcm2);
};

inline uint8_t Tool::combine(uint8_t pcm1, uint8_t pcm2) {
  uint16_t combine = pcm1 + pcm2;

  if (combine > UINT8_MAX)
    return UINT8_MAX;

  return (uint8_t)combine;
}

inline int16_t Tool::combine(int16_t pcm1, int16_t pcm2) {
  int32_t combined = pcm1 + pcm2;

  if (combined > INT16_MAX)
    return INT16_MAX;

  if (combined < INT16_MIN)
    return INT16_MIN;

  return (int16_t)combined;
}

template <class X> inline X Tool::combine(const X &pcm1, const X &pcm2) {
  X result;
  auto itr1 = begin(pcm1);
  auto itr2 = begin(pcm2);
  while (itr1 != end(pcm1) || itr2 != end(pcm2)) {
    auto first = (itr1 == end(pcm1) ? 0 : *itr1);
    auto second = (itr2 == end(pcm2) ? 0 : *itr2);
    result.push_back(combine(first, second));
    ++itr1;
    ++itr2;
  }
  return std::move(result);
}

inline std::string Tool::combine_16_samplebits(const std::string &pcm1,
                                               const std::string &pcm2) {
  std::string result;
  const int16_t *p1 = (const int16_t *)pcm1.c_str();
  size_t len1 = pcm1.size() / 2;
  const int16_t *p2 = (const int16_t *)pcm2.c_str();
  size_t len2 = pcm2.size() / 2;
  for (size_t i = 0, j = 0; i < len1 || j < len2; ++i, ++j) {
    int16_t first = (i < len1 ? p1[i] : 0);
    int16_t second = (j < len2 ? p2[j] : 0);
    int16_t combined = combine(first, second);
    result.append((char *)&combined, 2);
  }
  return result;
}

} // namespace wav

MYSPACE_END
