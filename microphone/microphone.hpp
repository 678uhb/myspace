#pragma once
#include "myspace/_/stdafx.hpp"

#if defined(MYSPACE_WINDOWS)
#include "myspace/critical/critical.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/wav/wav.hpp"
MYSPACE_BEGIN
class Microphone {
  MYSPACE_EXCEPTION_DEFINE(MicError, myspace::Exception)
public:
  // get total number of microphones
  static size_t count();

  // get micro info
  static WAVEINCAPS getMicInfo(size_t no) noexcept(false);

  static std::string toString(const WAVEINCAPS &x);

  // open microphone
  Microphone(size_t no, const wav::Format &format, size_t voicelength,
             std::function<void(std::string &&voice)> callback) noexcept(false);

  // close microphone
  ~Microphone();

  std::string read(size_t length) noexcept(false);

private:
  static void CALLBACK callbackFunc(HWAVEIN hwavein, UINT uMsg,
                                    DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                    DWORD_PTR dwParam2);
  void prepareBufferProc(size_t no);

private:
  bool buffer_ready_ = false;
  bool stop_ = false;
  std::thread prepare_buffer_thread_;
  WAVEFORMATEX format_ = {0};
  size_t voicelength_ = 0;
  std::function<void(std::string &&voice)> callback_;
  std::mutex mtx_;
  std::condition_variable cond_;
  HWAVEIN wave_in_ = 0;
  std::deque<std::unique_ptr<WAVEHDR>> buffers_;
};

inline void Microphone::prepareBufferProc(size_t no) {
  try {

    MYSPACE_DEFER(stop_ = true; cond_.notify_all());
    auto n =
        ::waveInOpen(&wave_in_, no, &format_,
                     reinterpret_cast<DWORD_PTR>(&Microphone::callbackFunc),
                     (DWORD_PTR) nullptr, CALLBACK_FUNCTION);
    MYSPACE_THROW_IF_EX(MicError, MMSYSERR_NOERROR != n, " n = ", n);

    size_t buff_count =
        std::max((size_t)3, (size_t)format_.nAvgBytesPerSec / voicelength_);

    for (size_t i = 0; i < buff_count; ++i) {
      auto buff = std::make_unique<WAVEHDR>(WAVEHDR{0});

      buff->lpData = (LPSTR)::malloc(voicelength_);
      MYSPACE_THROW_IF_EX(MicError, !buff->lpData);
      buff->dwBufferLength = voicelength_;
      buff->dwBytesRecorded = 0;
      buff->dwFlags = 0;
      buff->dwLoops = 0;
      buff->dwUser = (DWORD_PTR)this;
      buff->lpNext = 0;

      MYSPACE_THROW_IF_EX(
          MicError,
          MMSYSERR_NOERROR !=
              ::waveInPrepareHeader(wave_in_, buff.get(), sizeof(*buff)));

      MYSPACE_THROW_IF_EX(
          MicError, MMSYSERR_NOERROR !=
                        ::waveInAddBuffer(wave_in_, buff.get(), sizeof(*buff)));

      buffers_.emplace_back(move(buff));
    }

    MYSPACE_THROW_IF_EX(MicError, MMSYSERR_NOERROR != ::waveInStart(wave_in_));

    while (!stop_) {
      MYSPACE_IF_LOCK(mtx_) {
        if (stop_)
          break;
        if (!buffer_ready_) {
          cond_.wait(__ul, [this]() { return buffer_ready_ || stop_; });
          continue;
        }
        buffer_ready_ = false;
      }

      for (auto &buffer : buffers_) {
        if (buffer->dwFlags & WHDR_DONE) {
          try {
            callback_(std::string(buffer->lpData, buffer->dwBytesRecorded));
          } catch (...) {
            MYSPACE_DEV_EXCEPTION();
          }

          {
            auto n =
                ::waveInPrepareHeader(wave_in_, buffer.get(), sizeof(*buffer));
            MYSPACE_THROW_IF_EX(MicError, MMSYSERR_NOERROR != n, "n = ", n);
          }
          {
            auto n = ::waveInAddBuffer(wave_in_, buffer.get(), sizeof(*buffer));
            MYSPACE_THROW_IF_EX(MicError, MMSYSERR_NOERROR != n, "n = ", n);
          }
        }
      }
    }
  } catch (...) {
    MYSPACE_DEV_EXCEPTION();
  }

  ::waveInReset(wave_in_);
  for (auto &buffer : buffers_) {
    ::waveInUnprepareHeader(wave_in_, buffer.get(), sizeof(*buffer));
    if (buffer->lpData)
      ::free(buffer->lpData);
  }
  ::waveInClose(wave_in_);
}

inline size_t Microphone::count() { return ::waveInGetNumDevs(); }
inline WAVEINCAPS Microphone::getMicInfo(size_t no) noexcept(false) {
  WAVEINCAPS cap;
  MYSPACE_THROW_IF_EX(MicError, MMSYSERR_NOERROR !=
                                    ::waveInGetDevCaps(no, &cap, sizeof(cap)));
  return cap;
}
inline Microphone::Microphone(
    size_t no, const wav::Format &t_format, size_t voicelength,
    std::function<void(std::string &&voice)> callback) noexcept(false)
    : voicelength_(voicelength), callback_(callback) {

  MYSPACE_THROW_IF_EX(MicError, !voicelength);
  MYSPACE_THROW_IF_EX(MicError, !callback);
  format_ = wav::convert(t_format);
  prepare_buffer_thread_ = std::thread([this, no]() { prepareBufferProc(no); });
}
inline Microphone::~Microphone() {
  stop_ = true;
  cond_.notify_all();
  if (prepare_buffer_thread_.joinable())
    prepare_buffer_thread_.join();
}

inline void CALLBACK Microphone::callbackFunc(HWAVEIN hwavein, UINT uMsg,
                                              DWORD_PTR dwInstance,
                                              DWORD_PTR dwParam1,
                                              DWORD_PTR dwParam2) {
  auto header = (LPWAVEHDR)dwParam1;
  auto mic = (header ? (Microphone *)header->dwUser : nullptr);
  if (!mic || !header)
    return;
  if (mic->stop_)
    return;
  switch (uMsg) {
  case WIM_OPEN: {
  } break;
  case WIM_CLOSE: {
  } break;
  case WIM_DATA: {
    MYSPACE_IF_LOCK(mic->mtx_) { mic->buffer_ready_ = true; }
    mic->cond_.notify_all();
  } break;
  }
}

inline std::string Microphone::toString(const WAVEINCAPS &x) {
  std::stringstream ss;
  ss << "dwFormats : " << x.dwFormats << std::endl;
  ss << "szPname : " << x.szPname << std::endl;
  ss << "vDriverVersion : " << x.vDriverVersion << std::endl;
  ss << "wChannels : " << x.wChannels << std::endl;
  ss << "wMid : " << x.wMid << std::endl;
  ss << "wPid : " << x.wPid << std::endl;
  return ss.str();
}

MYSPACE_END
#endif
