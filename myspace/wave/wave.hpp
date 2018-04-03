#pragma once

#include "myspace/myspace_include.h"

MYSPACE_BEGIN

enum AudioFormat {
	Pcm = 1,
	Alaw = 6,
	Ulaw = 7,
};



#pragma pack(push)
#pragma pack(1)
//typedef struct tWAVEFORMATEX
//{
//	WORD        wFormatTag;         /* format type */
//	WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
//	DWORD       nSamplesPerSec;     /* sample rate */
//	DWORD       nAvgBytesPerSec;    /* for buffer estimation */
//	WORD        nBlockAlign;        /* block size of data */
//	WORD        wBitsPerSample;     /* number of bits per sample of mono data */
//	WORD        cbSize;             /* the count in bytes of the size of */
//									/* extra information (after cbSize) */
//} WAVEFORMATEX, *PWAVEFORMATEX, NEAR *NPWAVEFORMATEX, FAR *LPWAVEFORMATEX;
struct WaveFormat {
	uint16_t	_format;
	uint16_t	_channel;
	uint32_t	_sample_rate;
	uint32_t	_bytes_rate;
	uint16_t	_align;
	uint16_t	_sample_bytes;
};

struct WaveHead {
	WaveHead(const WaveFormat& fmt)
		:_format(fmt) {

	}
	char			_riff[4] = { 'R','I','F','F' };
	uint32_t		_riff_size = sizeof(WaveHead) - 8;
	char			_wave[4] = { 'W','A','V','E' };
	char			_fmt[4] = { 'f','m','t',' ' };
	uint32_t		_fmt_size = 18;
	WaveFormat		_format;
	uint16_t		_ext = 0;
	char			_data[4] = { 'd','a','t','a' };
	uint32_t		_data_size = 0;
};
#pragma pack(pop)




MYSPACE_END
