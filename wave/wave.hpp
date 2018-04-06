#pragma once

#include "myspace/_/include.hpp"

MYSPACE_BEGIN

enum AudioFormat {
  Pcm = 1,
  Alaw = 6,
  Ulaw = 7,
};

#pragma pack(push)
#pragma pack(1)
// typedef struct tWAVEFORMATEX
//{
//	WORD        wFormatTag;         /* format type */
//	WORD        nChannels;          /* number of channels (i.e. mono,
// stereo...) */ 	DWORD       nSamplesPerSec;     /* sample rate */
// DWORD nAvgBytesPerSec;    /* for buffer estimation */ 	WORD
// nBlockAlign; /*
// block size of data */ 	WORD        wBitsPerSample;     /* number of
// bits per sample of mono data */ 	WORD        cbSize;             /* the
// count in bytes of the size of */
//									/* extra
// information (after cbSize) */ } WAVEFORMATEX, *PWAVEFORMATEX, NEAR
//*NPWAVEFORMATEX, FAR *LPWAVEFORMATEX;
struct WaveFormat {
  uint16_t format_;
  uint16_t channel_;
  uint32_t sample_rate_;
  uint32_t bytes_rate_;
  uint16_t align_;
  uint16_t sample_bytes_;
};

struct WaveHead {
  WaveHead(const WaveFormat &fmt) : format_(fmt) {}
  char riff_[4] = {'R', 'I', 'F', 'F'};
  uint32_t riff_size_ = sizeof(WaveHead) - 8;
  char wave_[4] = {'W', 'A', 'V', 'E'};
  char fmt_[4] = {'f', 'm', 't', ' '};
  uint32_t fmt_size_ = 18;
  WaveFormat format_;
  uint16_t ext_ = 0;
  char data_[4] = {'d', 'a', 't', 'a'};
  uint32_t data_size_ = 0;
};
#pragma pack(pop)

MYSPACE_END
