
#include "myspace/_/include.hpp"
#include "myspace/wave/wave.hpp"

MYSPACE_BEGIN

class WavFile {
public:
  WavFile(const string &filename, WaveFormat format);

  ~WavFile();

  WavFile &append(const char *voice, size_t length);

  WavFile &append(const string &voice);

private:
  void close();

  WavFile &resetHead();

  WavFile &addSize(size_t n);

  WaveHead head_;

  string filename_;
};

WavFile::WavFile(const string &filename, WaveFormat format)
    : head_(format), filename_(filename) {}

WavFile::~WavFile() {
  // close();
}

WavFile &WavFile::append(const char *voice, size_t length) {
  if (voice && length > 0) {
    addSize(length);

    resetHead();

    ofstream(filename_.c_str(), ios::out | ios::binary | ios::app)
        .write(voice, length)
        .flush();
  }
  return *this;
}

WavFile &WavFile::append(const string &voice) {
  return append(voice.c_str(), voice.size());
}

void WavFile::close() { resetHead(); }

WavFile &WavFile::resetHead() {
  fstream fs(filename_, ios::out | ios::binary);

  if (fs.is_open()) {
    fs.seekp(0);

    fs.write((char *)&head_, sizeof(head_)).flush();
  }

  return *this;
}

WavFile &WavFile::addSize(size_t n) {
  head_.riff_size_ += n;

  head_.data_size_ += n;

  return *this;
}

MYSPACE_END
