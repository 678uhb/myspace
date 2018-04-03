
#include "myspace/myspace_include.h"
#include "myspace/wave/wave.hpp"

MYSPACE_BEGIN

class WavFile 
{
public:

	WavFile(const string& filename, WaveFormat format)
		:_head(format), _filename(filename)
    {

	}

	~WavFile() 
    {
		//close();
	}

	WavFile& append(const char* voice, size_t length) 
    {
		if (voice && length > 0) 
        {
			addSize(length);

			resetHead();

			ofstream(_filename.c_str(), ios::out | ios::binary | ios::app).write(voice, length).flush();
		}
		return *this;
	}

	WavFile& append(const string& voice) 
    {
		return append(voice.c_str(), voice.size());
	}


private:

	void close() 
    {
		resetHead();
	}

	WavFile& resetHead()
    {
		fstream fs(_filename, ios::out | ios::binary);
		
        if (fs.is_open()) 
        {
			fs.seekp(0);

			fs.write((char*)&_head, sizeof(_head)).flush();
		}

		return *this;
	}

	WavFile& addSize(size_t n)
     {
		_head._riff_size += n;

		_head._data_size += n;

		return *this;
	}

	WaveHead		_head;

	string			_filename;
};


MYSPACE_END
