
#include "myspace/_/stdafx.hpp"
#include "myspace/wav/wav.hpp"

MYSPACE_BEGIN

namespace wav {

class File {
public:
  enum Mode {
    APPEND,
    TRUNCATE,
  };

public:
  // open
  File(const std::string &filename, Format format, Mode mode = TRUNCATE);

  // close
  ~File();

  // add voice
  File &append(const char *voice, size_t length);

  // add voice
  File &append(const std::string &voice);

private:
  void close();

  File &resetHead();

  File &addSize(size_t n);

  Head head_;

  std::string filename_;
};

inline File::File(const std::string &filename, Format format, Mode mode)
    : head_(format), filename_(filename) {
  if (mode == TRUNCATE) {
    std::ofstream(filename, std::ios::out | std::ios::trunc | std::ios::binary);
  }
}

inline File::~File() {
  // close();
}

inline File &File::append(const char *voice, size_t length) {
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
inline File &File::append(const std::string &voice) {
  return append(voice.c_str(), voice.size());
}
inline void File::close() { resetHead(); }
inline File &File::resetHead() {
  std::fstream fs(filename_, std::ios::in | std::ios::out | std::ios::binary |
                                 std::ios::ate);
  if (fs.is_open()) {
    fs.seekp(0);
    fs.write((char *)&head_, sizeof(head_)).flush();
  }
  return *this;
}
inline File &File::addSize(size_t n) {
  head_.riff_size_ += (uint32_t)n;
  head_.data_size_ += (uint32_t)n;
  return *this;
}
} // namespace wav

MYSPACE_END
