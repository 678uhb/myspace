#pragma once
// platform
#if defined(_WIN32) || defined(_WIN64)
#define this_platform_windows
#elif defined(__linux__)
#define this_platform_linux
#else
#define this_platform_unknown
#endif
// compiler
#if defined(_GNUC_)
#define this_compiler_gcc
#elif defined(_MSC_VER)
#define this_compiler_msvc _MSC_VER
#else
#define this_complier_unknown
#endif

// system headers
#if defined(this_platform_windows)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifdef NOMINMAX
#undef  NOMINMAX
#endif
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <direct.h>
#pragma comment(lib, "ws2_32.lib")
#elif defined(this_platform_linux)
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#else
#error unknown platform
#endif
// c headers
#include <cerrno>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <cctype>
//cpp headers
#include <bitset>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <fstream>
#include <functional>
#include <future>
#include <ios>
#include <iostream>
#include <list>
#include <string>
#include <sstream>
#include <system_error>
#include <thread>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>

// this space
#define this_space  _
namespace this_space {

	using namespace std;
	using namespace chrono;

	class scope_t {
	public:
		template<class f_t, class... a_t>
		scope_t(f_t&& f, a_t&&... args) :f_(bind(forward<f_t&&>(f), forward<a_t&&>(args)...)) {}
		~scope_t() { if (f_) f_(); }
		void dismiss() { f_ = nullptr; }
	private:
		function<void()> f_ = nullptr;
	};

	template<class throw_t>
	void throw_e(throw_t&& e) {
		if (current_exception())
			throw_with_nested(e);
		else
			throw e;
	}

	class exception_t : public runtime_error {
	public:
		exception_t(const char* file, int line, const string& desc)
			:runtime_error([&](...) {
			stringstream ss;
			ss << file << ":" << line << " " << desc;
			return move(ss.str());
		}()) {}
	};

#define __annoymous_t(type,token,line)	type token##line
#define _annoymous_t(type,line)  __annoymous_t(type,annoymous,line)
#define annoymous_t(type)   _annoymous_t(type, __LINE__)
#define scope(f) annoymous_t(scope_t)([&](){f;})
#define debug(fmt, ...) if( log_t::level() <= log_t::level_t::debug) { log_t::print(log_t::debug, __FILE__,__LINE__, fmt, ##__VA_ARGS__);  };
#define info(fmt, ...) if( log_t::level() <= log_t::level_t::info) { log_t::print(log_t::info, __FILE__,__LINE__, fmt, ##__VA_ARGS__);  };
#define warn(fmt, ...) if( log_t::level() <= log_t::level_t::warn) { log_t::print(log_t::warn, __FILE__,__LINE__, fmt, ##__VA_ARGS__);  };
#define error(fmt, ...) if( log_t::level() <= log_t::level_t::error) { log_t::print(log_t::error, __FILE__,__LINE__, fmt, ##__VA_ARGS__);  };
#define buildstr(x)	#x
#define _source_pos(file, line) file ":" buildstr(line)
#define source_pos	 _source_pos(__FILE__, __LINE__)
#define throw_syserror(ec) do { \
    throw_e(syserror_t(__FILE__,__LINE__,ec, geterrormsg(ec)));  \
} while(0)
#define throw_syserror_if(x) if((x)) do { \
	auto ec = getlasterror();  \
    throw_e(syserror_t(__FILE__,__LINE__,ec, geterrormsg(ec)));  \
} while(0)
#define throw_runtimerror(fmt, ...) do {			\
	va_list ap;										\
	va_start(ap, fmt);								\
	scope(va_end(ap));								\
	string desc = move(vformat(fmt, ap));			\
	throw_e(exception_t(__FILE__,__LINE__, desc));	\
} while(0)
#define throw_runtimerror_if(x) do {			\
	if(x){												\
		throw_e(exception_t(__FILE__, __LINE__, #x));	\
	}													\
} while(0)
#define catch_all() catch(...){print_exception();}
	

	template<size_t len = 1024>
	string vformat(const char* fmt, va_list ap) {
		if (len <= 1)
			return "";
		string ret;
		//auto buf = new_unique_array<char>(len);
		unique_ptr<char[]> buf(new char[len]);
		int n = ::vsnprintf(buf.get(), len, fmt, ap);
		if (n > 0)
			ret.assign(buf.get(), min(len, (size_t)n));
		return move(ret);
	}
	template<size_t len = 1024>
	string format(const char* fmt, ...) {
		if (len <= 1)
			return "";
		string ret;
		va_list ap;
		va_start(ap, fmt);
		scope(va_end(ap));
		return move(vformat<len>(fmt, ap));
	}



	class syserror_t : public system_error {
	public:
		syserror_t(const char* file, int line, int ec, const string& desc) 
			:system_error(error_code(ec, system_category()), [&](...) {
			stringstream ss;
			ss << file << ":" << line << " " << desc;
			return move(ss.str());
		}()) {}
	};

	

	template<class t, class... a>
	unique_ptr<t> new_unique(a&&... args) {
		while (true) {
			try {
				auto p = new t(forward<a&&>(args)...);
				if (p) return unique_ptr<t>(p);
			}
			catch (bad_alloc&) { this_thread::yield(); }
		}
	}
	template<class t>
	unique_ptr<t[]> new_unique_array(size_t n) {
		while (true) {
			try {
				auto p = new t[n];
				if (p) return unique_ptr<t[]>(p);
			}
			catch (bad_alloc&) { this_thread::yield(); }
		}
	}
	template<class t, class... a>
	shared_ptr<t> new_shared(a&&... args) {
		while (true) {
			try {
				auto p = new t(forward<a&&>(args)...);
				if (p) return shared_ptr<t>(p);
			}
			catch (bad_alloc&) { this_thread::yield(); }
		}
	}
	template<class t>
	shared_ptr<t[]> new_shared_array(size_t n) {
		while (true) {
			try {
				auto p = new t[n];
				if (p) return shared_ptr<t[]>(p);
			}
			catch (bad_alloc&) { this_thread::yield(); }
		}
	}

	template<class = void>
	string& tolower(string& src) { for (auto& c : src) c = std::tolower(c); return src; }
	template<class = void>
	string& toupper(string& src) { for (auto& c : src) c = std::toupper(c); return src; }

	template<class = void>
	list<string> cut(string& src, const string& tokens) {
		list<string> ret;
		do {
			auto pos = src.find_first_of(tokens);
			if (pos == src.npos) break;
			ret.emplace_back(move(src.substr(0, pos)));
			src.erase(0, pos + 1);
		} while (true);
		return move(ret);
	}

	template<class = void>
	string& lstrip(string& src, const string& token = " \r\n\t") {
		return src.erase(0,src.find_first_not_of(token));
	}

	template<class = void>
	string& rstrip(string& src, const string& token = " \r\n\t") {
		return src.erase(src.find_last_not_of(token)+1 );
	}

	template<class type_t = string>
	type_t& strip(type_t& src, const string& token = " \r\n\t") {
		return lstrip(rstrip(src, token), token);
	}

	template<>
	list<string>& strip<list<string>>(list<string>& src, const string& token) {
		for (auto itr = src.begin(); itr != src.end(); ) {
			if (strip(*itr).empty())
				src.erase(itr++);
			else
				++itr;
		}
		return src;
	}
	template<class = void>
	list<string> strip(list<string>&& src, const string& token = "\r\n\t ") {
		for (auto itr = src.begin(); itr != src.end(); ) {
			if (strip(*itr).empty())
				src.erase(itr++);
			else
				++itr;
		}
		return move(src);
	}


	template<class = void>
	list<string> split(const string& src, const string& tokens = " \r\n\t") {
		list<string> ret;
		string tmp = src;
		while (!tmp.empty()) {
			auto pos = tmp.find_first_of(tokens);
			string subs = move(tmp.substr(0, pos));
			if(!subs.empty()){
				ret.emplace_back(move(subs));
			}
			tmp.erase(0, pos + 1);
		}
		return move(ret);
	}

	
	template<class = void>
	string strftime(time_t t, const string& fmt = "%F %T") {
		auto buf = new_unique_array<char>(128);
		struct tm _tm;
#ifdef this_platform_windows
		::localtime_s(&_tm, &t);
#else
		::localtime_r(&t, &_tm);
#endif
		string ret;
		auto n = ::strftime(buf.get(), 128, fmt.c_str(), &_tm);
		if (n > 0)
			ret.assign(buf.get(), n);
		return move(ret);
	}

#define if_lock(mtx)  if(  auto _ul = [](mutex& m){ return move(unique_lock<mutex>(m)); }(mtx) )

	template<class hold_t, class mtx_t = mutex, class cond_t = condition_variable>
	class shared_t {
	public:
		hold_t hold_;
		mtx_t mtx_;
		cond_t cond_;
	};

	template<class = void>
	int getlasterror() {
#ifdef this_platform_windows
		int ec = ::WSAGetLastError();
		if (ec) return ec;
		return ::GetLastError();
#else
		return errno;
#endif
	}
	template<class  = void>
	string geterrormsg(int ec) {
#ifdef this_platform_windows
		LPVOID buf = nullptr;
		scope(::LocalFree(buf));
		::FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			nullptr, ec, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&buf, 0, nullptr);
		return move(string((char*)buf));
#else
		auto buf = new_unique_array<char>(1024);
		return move(string(::strerror_r(ec, buf.get(), 1024)));
#endif
	}

	template<class = void>
	string getlasterrormsg() {
		return move(geterrormsg(getlasterror()));
	}

	class log_t {
		log_t() = delete;
	public:
		enum level_t { debug, info, warn, error };
	public:
		static level_t level() { return get_static().lv_;}
		static level_t level(level_t lv) { scope(get_static().lv_ = lv); return get_static().lv_; }
		static void log2console(bool f) { get_static().log2console_ = f; }
		static void log2file(bool f) { get_static().log2file_ = f; }
		static void logdir(const string& path) { get_static().lgodir_ = path; get_static().log2file_ = true; }
		static void print(level_t lv, const char* file, int line, const char* fmt, ...) {
			if (!get_static().log2file_ && !get_static().log2console_)
				return;
			
			va_list ap;
			va_start(ap, fmt);
			string msg(this_space::vformat<1024>(fmt, ap));
			va_end(ap);
			if (msg.back() != '\n')
				msg.append(1, '\n');
			stringstream ss;
			switch (lv) {
			case debug:  ss << "[debug]"; break;
			case info: ss << "[info]"; break;
			case warn: ss << "[warn]"; break;
			case error: ss << "[error]"; break;
			}
			ss << "[" << this_space::strftime(time(0)) << "]"
				<< "[" << this_thread::get_id() << "]"
				<< "[" << file << ":" << line << "]" << msg;

			if_lock(get_static().logqueue_.mtx_) {
				if (get_static().logqueue_.hold_.size() > 10000) {
					size_t x = 0;
					while (get_static().logqueue_.hold_.size() > 10000) {
						get_static().logqueue_.hold_.pop_front();
						++x;
					}
					stringstream ss;
					ss << "[warn][" << this_space::strftime(time(0)) << "][" << this_thread::get_id()
						<< "][" source_pos "]" << format("too much log to sink, truncate %u logs", x);
					get_static().logqueue_.hold_.emplace_back(move(ss.str()));
				}
				get_static().logqueue_.hold_.emplace_back(move(ss.str()));
			}
			get_static().logqueue_.cond_.notify_one();

			call_once(get_static().onceflag_, []() {
				thread([]() {
					ofstream ofs(get_static().lgodir_ + "log", ios::out|ios::app);
					while (true) {
						try {
							list<string> q;
							if_lock(get_static().logqueue_.mtx_) {
								if (get_static().logqueue_.hold_.empty()) {
									get_static().logqueue_.cond_.wait(_ul, []() { return !get_static().logqueue_.hold_.empty(); });
									continue;
								}
								q.swap(get_static().logqueue_.hold_);
							}
							while (!q.empty()) {
								if (get_static().log2console_) {
									cout << q.front();
								}

								auto size = ofs.tellp();
								if (size > (100 << 20)) {
									ofs.close();
									for (int i = 100; i >= 0; --i) {
										stringstream src;
										stringstream dst;
										if (i == 0) {
											src << get_static().lgodir_ << "log";
										}
										else {
											src << get_static().lgodir_ << "log." << i;
										}
										dst << get_static().lgodir_ << "log." << i + 1;
										::remove(dst.str().c_str());
										::rename(src.str().c_str(), dst.str().c_str());
									}
									ofs.open(get_static().lgodir_ + "log",ios::out | ios::trunc);
								}
								ofs.write(q.front().c_str(), q.front().size());
								if (q.size() < 100)
									ofs.flush();
								q.pop_front();
							}
						}
						catch (...) {

						}
					}
				}).detach();
			});
		}
	private:
		class log_static_t {
		public:
			level_t lv_ = debug;
			bool log2console_ = true;
			bool log2file_ = false;
			string lgodir_;
			once_flag onceflag_;
			shared_t<list<string>> logqueue_;
		};
	private:
		static log_static_t& get_static() {
			static log_static_t static_;
			return static_;
		}
	};
	template<class = void>
	void print_exception(exception& e, string& buf) {
		try {
			buf.append(e.what());
			if (!buf.empty() && buf.back() != '|')
				buf.append(1, '|');
			rethrow_if_nested(e);
		}
		catch (exception& e) {
			print_exception(e, buf);
		}
		catch (...) {
			buf.append("unknown exception|");
		}
	}
	template<class = void>
	void print_exception() {
		string buf;
		try {
			if (current_exception())
				rethrow_exception(current_exception());
		}
		catch (exception& e) {
			print_exception(e, buf);
		}
		catch (...) {
			buf.append("unknown exception|");
		}
		warn("%s", buf.c_str());
	}
	class config_t {
	public:
		config_t(const string& path) {
			string line;
			string section = "default";
			for (ifstream fs(path); getline(fs, line); ) {
				if (strip(line).empty())continue;
				if (line[0] == '[') {
					section = line.substr(1, line.find_last_not_of(']'));
				}
				else {
					auto pos = line.find_first_of('=');
					string key = move(line.substr(0, pos));
					string value = move(line.substr(pos+1));
					maps_[section][move(key)] = value;
				}
			}
		}
	public:
		template<class type_t = string>
		type_t get(const string& sk) {
			class find_t {};
			string s, k, v;
			try {
				if (sk.find("default") == string::npos) {
					auto pos = sk.find_first_of('.');
					if (pos != string::npos) {
						s = move(sk.substr(0, pos));
						k = move(sk.substr(pos + 1));
						auto itr = maps_.find(s);
						if (itr != maps_.end()) {
							auto iitr = itr->second.find(k);
							if (iitr != itr->second.end()) {
								v = iitr->second;
								throw find_t();
							}
						}
					}
				}
				auto itr = maps_["default"].find(sk);
				if (itr != maps_["default"].end()) {
					v = itr->second;
					throw find_t();
				}
				throw_runtimerror("%s configure not found", sk.c_str());
			}
			catch (find_t&) {
				stringstream ss;
				ss << v;
				type_t x;
				ss >> x;
				return move(x);
			}
		}
		template<class type_t = string>
		type_t get(const string& sk, const type_t& def) {
			try {
				return get<type_t>(sk);
			}
			catch (runtime_error&) {
				return move(type_t(def));
			}
		}
	private:
		unordered_map<string, unordered_map<string, string>> maps_;
	};

	template<class = void>
	void setblock(int fd, bool f) {
#ifdef this_platform_windows
		unsigned long ul = (f ? 0 : 1);
		throw_syserror_if(0 != ::ioctlsocket(fd, FIONBIO, (unsigned long *)&ul));
#else
		auto flag = ::fcntl(fd, F_GETFL, 0);
		throw_syserror_if(flag < 0);

		if (f){
			if ( flag & O_NONBLOCK )
				throw_syserror_if(0 != ::fcntl(fd, F_SETFL, (flag & ~O_NONBLOCK)));
		}
		else{
			if(!( flag & O_NONBLOCK))
				throw_syserror_if(0 != ::fcntl(fd, F_SETFL, flag | O_NONBLOCK));
		}
#endif
	}

	template<class = void>
	void closesock(int fd) {
#ifdef this_platform_windows
		while (WSAEWOULDBLOCK == ::closesocket(fd)) {
			setblock(fd, true);
			this_thread::yield();
		}
#else
		::close(fd);
#endif
	}

	class socketstream_t {
	public:
		socketstream_t(int fd) : fd_(fd) {

		}
		socketstream_t(const string& ip, uint16_t port) {
			fd_ = (int)::socket(AF_INET, SOCK_STREAM, 0);
			scope_t closefd( [this]() {close();});
			throw_syserror_if(fd_ < 0);
			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			throw_syserror_if(1 != ::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr));
			throw_syserror_if(0 != ::connect(fd_, (sockaddr*)&addr, sizeof(addr)));
			closefd.dismiss();
		}
		~socketstream_t() {
			close();
		}
	public:
		int getfd() {
			return fd_;
		}
		void writeall(const string& buf) {
			size_t sendn = 0;
			while (sendn < buf.size()) {
				setblock(fd_, true);
				sendn += write(buf.substr(sendn));
			}	
		}
		size_t write(const string& buf) {
			throw_runtimerror_if(fd_ < 0);
			size_t writen = 0;
			while (true) {
				if (writen >= buf.size())
					return writen;
				int n = ::send(fd_, buf.c_str() + writen, int(buf.size() - writen), 0);
				if (n <= 0) {
					if (writen > 0)
						return writen;
					int ec = getlasterror();
					switch (ec) {
					case EINTR:
					case EAGAIN:
#if !defined(this_platform_linux)
					case EWOULDBLOCK:
#endif
#ifdef this_platform_windows
					case WSAEWOULDBLOCK:
#endif
						return writen;
					default:
						throw_syserror(ec);
					}
				}
				writen += (size_t)n;
			}
		}
		string read(size_t count = 0, bool block = false) {
			throw_runtimerror_if(fd_ < 0);
			if (count == 0) block = false;
			setblock(fd_, block);
			string ret;
			static const size_t buflen = (10 << 10);
			size_t recved = 0;
			auto buf = new_unique_array<char>(buflen);
			while (true) {
				if (count && recved >= count)
					return move(ret);
				int n = ::recv(fd_, buf.get(), int(count ? min(buflen, count - recved) : buflen), 0);
				if (n <= 0) {
					if (!ret.empty())
						return move(ret);
					throw_syserror_if(n == 0);
					int ec = getlasterror();
					switch (ec) {
					case EINTR:
					case EAGAIN:
#if !defined(this_platform_linux)
					case EWOULDBLOCK:
#endif
#ifdef this_platform_windows
					case WSAEWOULDBLOCK:
#endif
						return move(ret);
					default:
						throw_syserror(ec);
					}
				}
				recved += (size_t)n;
				ret.append(buf.get(), n);
			}
		}

		void close() {
			if (fd_ >= 0) {
				closesock(fd_);
				fd_ = -1;
			}
		}
	private:
		int fd_ = -1;
	};


	class listenstream_t {
	public:
		listenstream_t(uint16_t port) {
			fd_ = (int)socket(AF_INET, SOCK_STREAM, 0);
			throw_runtimerror_if(fd_ < 0);
			scope_t closefd([this]() {close(); });
			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
			throw_syserror_if(0 != ::bind(fd_, (sockaddr*)&addr, sizeof(addr)));
			throw_syserror_if(0 != ::listen(fd_, 1024));
			closefd.dismiss();
		}
		~listenstream_t() {
			close();
		}
	public:
		int getfd() {
			return fd_;
		}
		void close() {
			if (fd_ >= 0) {
				closesock(fd_);
				fd_ = -1;
			}
		}
		shared_ptr<socketstream_t> accept() {
			sockaddr_in addr;
			socklen_t addrlen = sizeof(addr);
			int n = (int)::accept(fd_, (sockaddr*)&addr, &addrlen);
			if (n <= 0) {
				int ec = getlasterror();
				switch (ec) {
				case EINTR:
				case EAGAIN:
#if !defined(this_platform_linux)
				case EWOULDBLOCK:
#endif
#ifdef this_platform_windows
				case WSAEWOULDBLOCK:
#endif
					return nullptr;
				default:
					throw_syserror(ec);
				}
			}
			return new_shared<socketstream_t>(n);
		}
	private:
		int fd_ = -1;
	};


	class pump_t {
	public:
		class cmd_pop_t{
		};
		class cmd_stop_t {
		};
	public:
		~pump_t() {
			stop();
		}
	public:
		void stop() {
			stop_ = true;
			cond_.notify_one();
		}
		pump_t& push(shared_ptr<socketstream_t> l, function<void()> callback) {
			setblock(l->getfd(), false);
			if_lock(mtx_) {
				sockstreams_[l] = callback;
			}
			return *this;
		}
		pump_t& push(shared_ptr<listenstream_t> l, function<void()> callback) {
			setblock(l->getfd(), false);
			if_lock(mtx_) {
				listenstreams_[l] = callback;
			}
			return *this;
		}
		pump_t& run(bool forever = false) {
			while (!stop_) {
				fd_set rfds, wfds, efds;
				FD_ZERO(&rfds);
				FD_ZERO(&wfds);
				FD_ZERO(&efds);
				int maxfd = 0;
				decltype(listenstreams_) l;
				decltype(sockstreams_) s;
				if_lock(mtx_) {
					if (listenstreams_.empty() && sockstreams_.empty()) {
						if(!forever)
							return *this;
						else {
							cond_.wait(_ul, [this]() { return !listenstreams_.empty() || !sockstreams_.empty() || stop_; });
							continue;
						}
					}
					l.swap(listenstreams_);
					s.swap(sockstreams_);
				}
				scope(if_lock(mtx_) {
					for (auto x : l)
						listenstreams_.insert(x);
					for (auto x : s)
						sockstreams_.insert(x);
				});

				decltype(l) cl;
				decltype(s) cs;
				scope(
					for (auto x : cl) l.erase(x.first);
				for (auto x : cs) s.erase(x.first);
				);

				for (auto& p : l) {
					if (!p.first || !p.second || p.first->getfd() < 0) {
						cl.insert(p);
					}
					else {
						setblock(p.first->getfd(), false);
						FD_SET(p.first->getfd(), &rfds);
						FD_SET(p.first->getfd(), &efds);
						maxfd = max(maxfd, p.first->getfd());
					}
				}
				for (auto& p : s) {
					if (!p.first || !p.second || p.first->getfd() < 0) {
						cs.insert(p);
					}
					else {
						setblock(p.first->getfd(), false);
						FD_SET(p.first->getfd(), &rfds);
						FD_SET(p.first->getfd(), &efds);
						maxfd = max(maxfd, p.first->getfd());
					}
				}
				timeval tv = { 1, 0 };
				int n = ::select(maxfd + 1, &rfds, &wfds, &efds, &tv);
				throw_syserror_if(n < 0);
				
				for (auto& p : l) {
					if (FD_ISSET(p.first->getfd(), &rfds)
						|| FD_ISSET(p.first->getfd(), &efds)) {
						try {
							p.second();
						}
						catch (cmd_pop_t&) {
							cl.insert(p);
						}
						catch (cmd_stop_t&) {
							return *this;
						}
					}
				}
				for (auto& p : s) {
					if (FD_ISSET(p.first->getfd(), &rfds)
						|| FD_ISSET(p.first->getfd(), &efds)) {
						try {
							p.second();
						}
						catch (cmd_pop_t&) {
							cs.insert(p);
						}
						catch (cmd_stop_t&) {
							return *this;
						}
					}
				}
			}
			return *this;
		}
	private:
		bool stop_ = false;
		mutex mtx_;
		condition_variable cond_;
		unordered_map<shared_ptr<socketstream_t>, function<void()>> sockstreams_;
		unordered_map<shared_ptr<listenstream_t>, function<void()>> listenstreams_;
	};

#ifdef this_platform_windows
	class net_switch_t {
	public:
		net_switch_t() { throw_syserror_if(0 != ::WSAStartup(MAKEWORD(1, 1), &wsadata_)); }
		~net_switch_t() { throw_syserror_if(0 != ::WSACleanup()); }
	private:
		WSAData wsadata_;
	};
#define net_switch() annoymous_t(net_switch_t);
#else
	class net_switch_t {

	};
#define net_switch()
#endif

	class transaction_t {
	public:
		transaction_t(function<void()> j)
		:job_(j){

		}
		~transaction_t() {
			dismiss();
		}
		void dismiss() {
			while (!dependences_.empty()) {
				auto x = dependences_.front();
				dependences_.pop_front();
				if(x) x->dismiss();
			}
			if (job_) {
				job_ = nullptr;
			}
		}
	public:
		transaction_t& start() {
			while (!dependences_.empty()) {
				auto t = dependences_.front();
				dependences_.pop_front();
				decltype(job_) x;
				x.swap(job_);
				scope_t dojob([this, &x]() {
					job_.swap(x);
				});
				if (t) {
					t->start();
				}
				else {
					dojob.dismiss();
				}
			}
			if (job_) {
				decltype(job_) x;
				x.swap(job_);
				x();
			}
			return *this;
		}
		transaction_t& depends(shared_ptr<transaction_t> t) {
			dependences_.push_back(t);
			return *this;
		}
	private:
		function<void()> job_;
		list<shared_ptr<transaction_t>> dependences_;
	};

	class thread_pool_t {
	public:
		thread_pool_t(size_t count = thread::hardware_concurrency()) {
			for (size_t x = 0; x < count; ++x) {
				threads_.emplace_back([this]() {
					while (!stop_) {
						function<void()> job;
						if_lock(jobs_.mtx_) {
							if (jobs_.hold_.empty()) {
								jobs_.cond_.wait(_ul, [this]() {
									return !jobs_.hold_.empty() || stop_;
								});
								continue;
							}
							job.swap(jobs_.hold_.front());
							jobs_.hold_.pop_front();
						}
						if (job) {
							job();
						}
					}
				});
			}
		}
		~thread_pool_t() {
			stop_ = true;
			jobs_.cond_.notify_all();
			for (auto& t : threads_) {
				if (t.joinable())t.join();
			}
		}
	public:
		template<class ft, class... argst>
		auto push_front( ft&& f, argst&&... args )
		-> future<typename result_of<ft(argst...)>::type> {
			using return_t = typename result_of<ft(argst...)>::type;
			auto job = make_shared<packaged_task<return_t()>>(
				bind(forward<ft>(f), forward<argst>(args)...));
			auto ret = job->get_future();
			if_lock(jobs_.mtx_) {
				jobs_.hold_.emplace_front([job]() { (*job)(); });
			}
			jobs_.cond_.notify_one();
			return ret;
		}
		template<class ft, class... argst>
		auto push_back(ft&& f, argst&&... args)
			-> future<typename result_of<ft(argst...)>::type> {
			using return_t = typename result_of<ft(argst...)>::type;
			auto job = make_shared<packaged_task<return_t()>>(
				bind(forward<ft>(f), forward<argst>(args)...));
			auto ret = job->get_future();
			if_lock(jobs_.mtx_) {
				jobs_.hold_.emplace_back([job]() { (*job)(); });
			}
			jobs_.cond_.notify_one();
			return ret;
		}
	private:
		bool stop_ = false;
		shared_t < list<function<void()>>> jobs_;
		list<thread> threads_;
	};
}
using namespace this_space;
