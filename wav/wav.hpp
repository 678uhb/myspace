#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

namespace wav {

enum AudioFormat {
  PCM = 1,
  ALAW = 6,
  ULAW = 7,
};

#pragma pack(push)
#pragma pack(1)
struct Format {
  uint16_t format_ = AudioFormat::PCM;
  uint16_t channels_ = 1;
  uint32_t sample_rate_ = 16000;
  uint32_t bytes_rate_ = 32000;
  uint16_t align_ = 2;
  uint16_t sample_bits_ = 16;
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
struct Head {
  Head(const wav::Format &fmt) : format_(fmt) {}
  char riff_[4] = {'R', 'I', 'F', 'F'};
  uint32_t riff_size_ = sizeof(wav::Head) - 8;
  char wave_[4] = {'W', 'A', 'V', 'E'};
  char fmt_[4] = {'f', 'm', 't', ' '};
  uint32_t fmt_size_ = 18;
  wav::Format format_;
  uint16_t ext_ = 0;
  char data_[4] = {'d', 'a', 't', 'a'};
  uint32_t data_size_ = 0;
};
#pragma pack(pop)

#if defined(MYSPACE_WINDOWS)
inline WAVEFORMATEX convert(const Format &src) {
  WAVEFORMATEX dst;
  dst.wFormatTag = src.format_;
  dst.nChannels = src.channels_;
  dst.nSamplesPerSec = src.sample_rate_;
  dst.wBitsPerSample = src.sample_bits_;
  dst.nAvgBytesPerSec =
      dst.nSamplesPerSec * dst.nChannels * dst.wBitsPerSample / 8;
  dst.cbSize = sizeof(dst);
  dst.nBlockAlign = dst.nChannels * dst.wBitsPerSample / 8;
  return dst;
}
#endif

} // namespace wav

MYSPACE_END
