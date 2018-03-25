#pragma once

#include "myspace/config.hpp"

MYSPACE_BEGIN

class Wav
{

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
struct wave_format_t {
	uint16_t	_format;
	uint16_t	_channel;
	uint32_t	_sample_rate;
	uint32_t	_bytes_rate;
	uint16_t	_align;
	uint16_t	_sample_bytes;
};

struct wave_head_t {
	wave_head_t(const wave_format_t& fmt)
		:_format(fmt) {

	}
	char			_riff[4] = { 'R','I','F','F' };
	uint32_t		_riff_size = sizeof(wave_head_t) - 8;
	char			_wave[4] = { 'W','A','V','E' };
	char			_fmt[4] = { 'f','m','t',' ' };
	uint32_t		_fmt_size = 18;
	wave_format_t	_format;
	uint16_t		_ext = 0;
	char			_data[4] = { 'd','a','t','a' };
	uint32_t		_data_size = 0;
};
#pragma pack(pop)



class wave_file_t {
public:

	wave_file_t(const string& filename, wave_format_t format)
		:_head(format), _filename(filename) {
	}

	~wave_file_t() {
		//close();
	}

	wave_file_t& write(const char* s, size_t l) {
		if (s && l > 0) {
			add_size(l);
			reset_head();
			ofstream(_filename.c_str(), ios::out | ios::binary | ios::app).write(s, l).flush();
		}
		return *this;
	}
	wave_file_t& write(const string& str) {
		return write(str.c_str(), str.size());
	}


private:

	void close() {
		reset_head();
	}

	wave_file_t& reset_head() {
		fstream fs(_filename, ios::out | ios::binary);
		if (fs.is_open()) {
			fs.seekp(0);
			fs.write((char*)&_head, sizeof(_head));
		}
		return *this;
	}

	wave_file_t& add_size(size_t n) {
		_head._riff_size += n;
		_head._data_size += n;
		return *this;
	}

	wave_head_t		_head;
	string			_filename;
};




MYSPACE_END
