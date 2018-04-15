
#include "myspace/_/stdafx.hpp"
#include "myspace/wave/wave.hpp"

MYSPACE_BEGIN

class WavFile {
public:
  WavFile(const std::string &filename, WaveFormat format);

  ~WavFile();

  WavFile &append(const char *voice, size_t length);

  WavFile &append(const std::string &voice);

private:
  void close();

  WavFile &resetHead();

  WavFile &addSize(size_t n);

  WaveHead head_;

  std::string filename_;
};

inline WavFile::WavFile(const std::string &filename, WaveFormat format)
    : head_(format), filename_(filename) {}

inline WavFile::~WavFile() {
  // close();
}

inline WavFile &WavFile::append(const char *voice, size_t length) {
  if (voice && length > 0) {
    addSize(length);

    resetHead();

    std::ofstream(filename_.c_str(),
                  std::ios::out | std::ios::binary | std::ios::app)
        .write(voice, length)
        .flush();
  }
  return *this;
}
inline WavFile &WavFile::append(const std::string &voice) {
  return append(voice.c_str(), voice.size());
}
inline void WavFile::close() { resetHead(); }
inline WavFile &WavFile::resetHead() {
  std::fstream fs(filename_, std::ios::out | std::ios::binary);

  if (fs.is_open()) {
    fs.seekp(0);

    fs.write((char *)&head_, sizeof(head_)).flush();
  }

  return *this;
}
inline WavFile &WavFile::addSize(size_t n) {
  head_.riff_size_ += n;

  head_.data_size_ += n;

  return *this;
}

MYSPACE_END
