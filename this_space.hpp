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
#ifndef NOMINMAX
#define  NOMINMAX
#endif
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <direct.h>
#include <corecrt_io.h>
#include <Objbase.h>
#pragma comment(lib, "ws2_32.lib")
#elif defined(this_platform_linux)
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <uuid/uuid.h>
#else
#error unknown platform
#endif
// c headers
#include <cerrno>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <cctype>
#include <csetjmp>
//cpp headers
#include <algorithm>
#include <bitset>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <fstream>
#include <functional>
#include <future>
#include <ios>
#include <iostream>
#include <iomanip>
#include <list>
#include <string>
#include <sstream>
#include <system_error>
#include <thread>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>
#include <tuple>
#include <deque>
// this space
#define this_space  _
namespace this_space {

	using namespace std;
	using namespace chrono;
	using namespace this_thread;

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

	class scope_t {
	public:
		template<class f_t, class... a_t>
		scope_t(f_t&& f, a_t&&... args) :_f(bind(forward<f_t&&>(f), forward<a_t&&>(args)...)) {}
		~scope_t() { if (_f) try { _f(); } catch (...) {}; }
		void dismiss() { _f = nullptr; }
	private:
		function<void()> _f = nullptr;
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
	

	template<size_t len = 1024>
	string vformat(const char* fmt, va_list ap) {
		if (len <= 1)
			return "";
		string ret;
		auto buf = new_unique_array<char>(len);
		//unique_ptr<char[]> buf(new char[len]);
		int n = ::vsnprintf(buf.get(), len, fmt, ap);
		if (n > 0)
			ret.assign(buf.get(), min(len, (size_t)n));
		return move(ret);
	}
template<class = void>
void throw_runtimerror_impl(const char* file, int line, const char* fmt, ...) {
	va_list ap;										
		va_start(ap, fmt);						
		scope(va_end(ap));								
		string desc = move(vformat(fmt, ap));			
		throw_e(exception_t(file, line, desc));	
}
#define throw_runtimerror(fmt, ...) throw_runtimerror_impl(__FILE__,__LINE__,fmt, ##__VA_ARGS__)
#define throw_runtimerror_if(x) do {			\
	if(x){												\
		throw_e(exception_t(__FILE__, __LINE__, #x));	\
	}													\
} while(0)
#define catch_all() catch(...){print_exception();}
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


	string basename(const string& path) {
		auto pos = path.find_last_of("\\/");
		if (pos != string::npos)
			return move(path.substr(pos + 1));
		return move(path);
	}
	
	size_t filesize(const string& path) {
		ifstream ifs(path, ios::in | ios::binary);
		if (!ifs.is_open())
			return 0;
		ifs.seekg(0, ios::end);
		return ifs.tellg();
	}

	template<class = void>
	string& tolower(string& src) { for (auto& c : src) c = std::tolower(c); return src; }
	template<class = void>
	string& toupper(string& src) { for (auto& c : src) c = std::toupper(c); return src; }

	template<class = void>
	list<string> lsplit(string& src, const string& tokens) {
		list<string> ret;
		do {
			auto pos = src.find_first_of(tokens);
			if (pos == src.npos) break;
			ret.emplace_back(move(src.substr(0, pos)));
			src.erase(0, pos + 1);
		} while (true);
		return move(ret);
	}


	string strip(const string& src, const string& token = "") {
		if (src.empty())
			return src;
		if (token.empty()) {
			size_t first = 0;
			for (auto c : src) {
				if (std::isblank(c) || std::iscntrl(c)) {
					first++;
					continue;
				}
				break;
			}
			size_t last = src.size();
			for (auto itr = src.rbegin(); itr != src.rend(); ++itr) {
				if (std::isblank(*itr) || std::iscntrl(*itr)) {
					last--;
					continue;
				}
				break;
			}
			return move(src.substr(first, last - first));
		}
		auto beginpos = src.find_first_not_of(token);
		auto endpos = src.find_last_not_of(token);
		return move(src.substr(beginpos, endpos - beginpos));
	}

	deque<string> split(const string& src, const string& tokens) {
		deque<string> ret;
		string tmp = src;
		for (size_t pos = src.find_first_of(tokens), last_pos = 0;
			last_pos != string::npos;
			last_pos = pos, pos = src.find_first_of(tokens, last_pos + tokens.size())) {
			ret.emplace_back(move(src.substr((last_pos == 0? 0 : last_pos + tokens.size()), pos)));
		}
		return move(ret);
	}


	deque<string> split(const string& src, char delm) {
		return move(split(src, string(1, delm)));
	}

	deque<string> split(const char* src, char delm) {
		return move(split(string(src), string(1, delm)));
	}

	
	
	template<class = void>
	string time_format(time_t t = time(0), const string& fmt = "%F %T") {
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

#define if_lock(mtx)  if(  auto __ul = [](mutex& m){ return move(unique_lock<mutex>(m)); }(mtx) )

	template<class hold_t, class mtx_t = mutex, class cond_t = condition_variable>
	class shared_t {
	public:
		hold_t _hold;
		mtx_t _mtx;
		cond_t _cond;
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
	string uuid() {
#ifdef this_platform_windows
		GUID guid;
		string uuid;
		::CoCreateGuid(&guid);
		{
			stringstream ss;
			ss << hex << setw(sizeof(guid.Data1)<<1) << setfill('0') << guid.Data1;
			uuid.append(move(ss.str()));
			uuid.append(1, '-');
		}
		{
			stringstream ss;
			ss << hex << setw(sizeof(guid.Data2) << 1) << setfill('0') << guid.Data2;
			uuid.append(move(ss.str()));
			uuid.append(1, '-');
		}
		{
			stringstream ss;
			ss << hex << setw(sizeof(guid.Data3) << 1) << setfill('0') << guid.Data3;
			uuid.append(move(ss.str()));
			uuid.append(1, '-');
		}
		for( auto i = 0; i < sizeof(guid.Data4); ++i)
		{
			stringstream ss;
			ss << hex << setw(2) << setfill('0') << (int)guid.Data4[i];
			uuid.append(move(ss.str()));
		}
		return move(uuid);
#endif
#ifdef this_platform_linux
		uuid_t uid;	
		uuid_generate(uid);
		stringstream ss;
		for(size_t i = 0; i < sizeof(uid); ++i)
			ss << hex << setw(2) << setfill('0') << (int)((char*)&uid)[0];
		return move(ss.str());
#endif
	}
	template<class = void>
	string getlasterrormsg() {
		return move(geterrormsg(getlasterror()));
	}


	class log_t {
	public:
		enum level_t {
			lv_debug = 0, 
			lv_info,
			lv_warn,
			lv_error
		};

		template<class... types>
		log_t& debug(const char* file, int line, const types&... args) {
			if(_lv >= lv_debug)
				print(lv_debug, file, line, args...);
			return *this;
		}

		template<class... types>
		log_t& info(const char* file, int line, const types&... args) {
			if (_lv >= lv_info)
				print(lv_info, file, line, args);
			return *this;
		}

		template<class... types>
		log_t& warn(const char* file, int line, const types&... args) {
			if (_lv >= lv_warn)
				print(lv_warn, file, line, args);
			return *this;
		}

		template<class... types>
		log_t& error(const char* file, int line, const types&... args) {
			if(_lv >= lv_error)
				print(lv_error, file, line, args);
			return *this;
		}

		log_t& set_level(level_t lv) { 
			_lv = lv;
			return *this;
		}
		
		level_t get_level() {
			return _lv;
		}
		
		log_t& set_log2console(bool f) {
			_log2console = f;
			return *this;
		}

		log_t& set_log2file(bool f) {
			_log2file = f;
			return *this;
		}
		
		log_t& set_logfile(const string& path) { 
			this->_logfile = path;
			return *this;
		}
	
	private:

		template<class... types>
		log_t& print(level_t lv, const char* file, int line, const types&... args) {
			
			if_lock(_mtx) {

				auto this_time = high_resolution_clock::now();
				scope(_lasttime = this_time);

				stringstream ss;
				ss << "[" << time_format(time(0), "%H:%M:%S");
				ss << "," << duration_cast<milliseconds>(this_time - _lasttime).count() << "]";
				switch (lv) {
				case lv_debug: ss << "[debug]"; break;
				case lv_info: ss << "[info]"; break;
				case lv_warn: ss << "[warn]"; break;
				case lv_error: ss << "[error]"; break;
				}
				ss << "[" << this_thread::get_id() << "]";
				ss << "[" << basename(file) << ":" << line << "]";

				_print(ss, args...);
				string log(move(ss.str()));

				if (!log.empty()) {
					if (log.back() != '\n')
						log.append(1, '\n');
					if (_log2console) {
						cout << log;
						cout.flush();
					}
					if (_log2file && !_logfile.empty()) {
						ofstream ofs(_logfile, ios::out | ios::binary | ios::app);
						if (!ofs.is_open()) {
							ofs.open(_logfile, ios::out | ios::binary | ios::trunc);
							ofs.close();
							ofs.open(_logfile, ios::out | ios::binary | ios::app);
						}
						if (ofs.is_open()) {
							if (filesize(_logfile) >= (100 << 20)) {
								ofs.open(_logfile, ios::out | ios::binary | ios::trunc);
								ofs.close();
								ofs.open(_logfile, ios::out | ios::binary | ios::app);
							}
							if(ofs.is_open())
								ofs.write(log.c_str(), log.size()).flush();
						}	
					}
				}
			}
			return *this;
		}

		template<class type, class... types>
		log_t& _print(stringstream& ss, const type& arg, const types&... args) {
			ss << arg;
			return _print(ss, args...);
		}

		log_t& _print(stringstream&) {
			return *this;
		}

		mutex								_mtx;
		bool								_log2console = true;
		bool								_log2file = false;
		level_t								_lv = lv_debug;
		string								_logfile;
		once_flag							_onceflag;
		shared_t<list<string>>				_logqueue;
		high_resolution_clock::time_point	_lasttime = high_resolution_clock::now();
	};

#define log_debug(log, ...) log.debug(__FILE__,__LINE__, ##__VA_ARGS__)
#define log_info(log, ...)	log.info(__FILE__,__LINE__, ##__VA_ARGS__)
#define log_warn(log, ...)	log.warn(__FILE__,__LINE__, ##__VA_ARGS__)
#define log_error(log, ...)	log.error(__FILE__,__LINE__, ##__VA_ARGS__)



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
			string section = "this";
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
				auto itr = maps_["this"].find(sk);
				if (itr != maps_["this"].end()) {
					v = itr->second;
					throw find_t();
				}
			}
			catch (find_t&) {
				stringstream ss;
				ss << v;
				type_t x;
				ss >> x;
				return move(x);
			}
			throw_runtimerror("%s configure not found", sk.c_str());
			return type_t();
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

	template<class type_t>
	class resource_t {
	public:
		resource_t(type_t x):res_(move(x)){}
		virtual ~resource_t() = 0;
		resource_t(resource_t<type_t>&& o)
		: res_(move(o.res_)) {}
		resource_t(const resource_t&) = delete;
	public:
		virtual type_t& get() = 0;
		virtual void close() = 0;
	protected:
		type_t res_;
	};
	template<class type_t>
	resource_t<type_t>::~resource_t() {

	}

	class fd_t : public resource_t<int> {
	public:
		fd_t(int fd = -1)
			:resource_t<int>(fd) {}
		~fd_t() {
			close();
		}
	public:
		virtual int& get() {
			return res_;
		}
		int getfd() {
			return res_;
		}
		virtual void close() {
			if (res_ >= 0)
#ifdef this_platform_windows
				::_close(res_);
#else
				::close(res_);
#endif
			res_ = -1;
		}
#ifndef this_platform_windows
		void setblock(bool f) {
			auto flag = ::fcntl(res_, F_GETFL, 0);
			throw_syserror_if(flag < 0);

			if (f) {
				if (flag & O_NONBLOCK)
					throw_syserror_if(0 != ::fcntl(res_, F_SETFL, (flag & ~O_NONBLOCK)));
			}
			else {
				if (!(flag & O_NONBLOCK))
					throw_syserror_if(0 != ::fcntl(res_, F_SETFL, flag | O_NONBLOCK));
			}
		}
#endif
	};

	class format_t {
	public:
		template<class type>
		format_t& operator << (const type& x) {
			_ss << x;
			return *this;
		}

		template <class type>
		operator type () {
			type tmp;
			_ss >> tmp;
			return move(tmp);
		}

	private:
		stringstream _ss;
	};

	class select_t {
	public:
		enum detect_type_t {
			detect_read = 1,
			detect_write = 1 << 1,
			detect_exception = 1 << 2,
		};

		template<class type>
		select_t& push(type x, detect_type_t dt = detect_read) {
			_maxfd = max(_maxfd, x->get_fd());
			if (dt & detect_read) FD_SET(x->get_fd(), &_r);
			if (dt & detect_write) FD_SET(x->get_fd(), &_w);
			if (dt & detect_exception) FD_SET(x->get_fd(), &_e);
			return *this;
		}

		select_t& wait(high_resolution_clock::duration duration) {
			timeval tv{ duration_cast<seconds>(duration).count(), duration_cast<microseconds>(duration).count() % 1000000 };
			::select(_maxfd + 1, &_r, &_w, &_e, &tv);
			this->reset();
			return *this;
		}
	private:
		select_t& reset() {
			FD_ZERO(&_r);
			FD_ZERO(&_w);
			FD_ZERO(&_e);
			_maxfd = -1;
			return *this;
		}
	private:
		int		_maxfd = -1;
		fd_set _r = { 0 };
		fd_set _w = { 0 };
		fd_set _e = { 0 };
	};

	class socket_t {
	public:
		socket_t(int sock) {
			_sock = sock;
		}


		socket_t(const string& addr, high_resolution_clock::duration timeout) {
			set_addr(addr.c_str()).connect(timeout);
		}

		socket_t(const string& addr, uint16_t port, high_resolution_clock::duration timeout) {
			set_addr(addr.c_str()).set_port(port).connect(timeout);
		}

		~socket_t() {
			close();
		}

		size_t send(const string& data, high_resolution_clock::duration timeout) {
			size_t sendn = 0;
			for (auto this_time = high_resolution_clock::now() , begin_time = this_time;
				sendn < data.size()  && this_time - begin_time <= timeout
				;this_time = high_resolution_clock::now())
			{
				auto n = ::send(_sock, data.c_str() + sendn, int(data.size() - sendn), 0);
				if (n > 0)
					sendn += n;
				else if (n == 0)
					break;
				else {
					auto e = WSAGetLastError();
					if (e == WSAEWOULDBLOCK | e == WSAEINTR | e == WSAEINPROGRESS) {
						auto sel = new_shared<select_t>();
						sel->push(this, select_t::detect_write);
						sel->wait(timeout - (this_time - begin_time));
						continue;
					}	
					break;
				}
			}
			return sendn;
		}

		string recv(high_resolution_clock::duration timeout) {
			string data;
			size_t buflen = 4096;
			auto buf = new_unique_array<char>(buflen);
			for (auto begin_time = high_resolution_clock::now(), this_time = begin_time;
				this_time - begin_time <= timeout;
				this_time = high_resolution_clock::now()) 
			{
				auto n = ::recv(_sock, buf.get(), (int)buflen, 0);
				if (n > 0)
					data.append(buf.get(), n);
				else if (n == 0)
					break;
				else {
					auto e = WSAGetLastError();
					if (e == WSAEWOULDBLOCK | e == WSAEINTR | e == WSAEINPROGRESS) 
					{
						auto sel = new_shared<select_t>();
						sel->push(this);
						sel->wait(timeout - (this_time - begin_time));
						continue;
					}
					break;
				}
			}
			return move(data);
		}

		string recv(size_t len, high_resolution_clock::duration timeout) {
			if (len == 0)
				return "";
			size_t recvn = 0;
			auto buf = new_unique_array<char>(len);
			for (auto this_time = high_resolution_clock::now(), begin_time = this_time;
				recvn < len && this_time - begin_time <= timeout;
				this_time = high_resolution_clock::now()) 
			{
				auto n = ::recv(_sock, buf.get() + recvn, int(len - recvn), 0);
				if (n > 0) {
					recvn += n;
				}
				else if (n == 0)
					break;
				else {
					auto e = WSAGetLastError();
					if (e == WSAEWOULDBLOCK | e == WSAEINTR | e == WSAEINPROGRESS)
					{
						auto sel = new_shared<select_t>();
						sel->push(this);
						sel->wait(timeout - (this_time - begin_time));
						continue;
					}
					break;
				}
			}
			return move(string(buf.get(),recvn));
		}

		string recv_until(const string& delm, high_resolution_clock::duration timeout) 
		{
			if (delm.empty())
				return move(recv(timeout));
			size_t matched = 0;
			size_t recvn = delm.size();
			string ret;
			auto buf = new_unique_array<char>(recvn);
			for (auto this_time = system_clock::now(), begin_time = this_time;
				(ret.size() < delm.size() || ret.rfind(delm, ret.size() - delm.size()) == ret.npos)
				&& this_time - begin_time <= timeout;
				this_time = system_clock::now()) 
			{
				auto n = ::recv(_sock, buf.get(), int(recvn), 0);
				
				if (n == 0)
					break;

				if( n < 0) {
					auto e = WSAGetLastError();
					if (e == WSAEWOULDBLOCK | e == WSAEINTR | e == WSAEINPROGRESS)
					{
						auto sel = new_shared<select_t>();
						sel->push(this);
						sel->wait(timeout - (this_time - begin_time));
						continue;
					}
					break;
				}

				ret.append(buf.get(), n);
				recvn = 0;
				for (size_t maxmatchn = std::min(delm.size(), ret.size()); maxmatchn; --maxmatchn)
				{
					if (ret.compare(ret.size() - maxmatchn, maxmatchn, delm, 0,maxmatchn) == 0)
					{
						recvn = delm.size() - maxmatchn;
						break;
					}
				}
				if (!recvn) recvn = delm.size();
			}
			return move(ret);
		}

		socket_t& setblock() {
			return setblock(true);
		}

		socket_t& setnonblock() {
			return this->setblock(false);
		}

		bool is_blocked() {
			return _isblocked;
		}

		operator bool() {
			return is_connected();
		}

		int get_fd() {
			return _sock;
		}

		bool is_connected() {
		again:
			char c;
			setnonblock();
			auto n = ::recv(_sock, &c, 1, MSG_PEEK);
			if (n == 0)
				return false;
			if (n > 0)
				return true;
			auto e = WSAGetLastError();
			switch (e) {
			case WSAEWOULDBLOCK:
			case WSAEINPROGRESS:
				return true;
			case WSAEINTR:
				goto again;
			default:
				return false;
			}
		}

		void close() {
			if (_sock >= 0) {
				setblock(true);
				while (WSAEWOULDBLOCK == ::closesocket(_sock)) {
					this_thread::sleep_for(milliseconds(100));
				}
				_sock = -1;
			}
		}

	private:
		

		socket_t& set_port(uint16_t port) {
			_port = port; return *this;
		}

		socket_t& set_addr(const char* ipport) {
			auto tokens = split(ipport, ':');
			if (tokens.size() == 1) {
				_ip = strip(tokens[0]);
			}
			else if (tokens.size() >= 2) {
				_ip = strip(tokens[0]);
				_port = (uint16_t)(format_t() << strip(tokens[1]));
			}
			return *this;
		}

		socket_t& connect(high_resolution_clock::duration timeout) {
			if (!_ip.empty() || _port != 0) {
				
				this->close();
				_sock = (int)::socket(AF_INET, SOCK_STREAM, 0);
				setnonblock();

				for ( auto this_time = high_resolution_clock::now(), begin_time = this_time ;
				  !is_connected() && this_time - begin_time <= timeout ;
					this_time = high_resolution_clock::now()) {
					
					sockaddr_in addr = { 0 };
					addr.sin_family = AF_INET;
					addr.sin_port = htons(_port);
					inet_pton(AF_INET, _ip.c_str(), &addr.sin_addr.s_addr);
					auto n = ::connect(_sock, (sockaddr*)&addr, sizeof(addr));

					if (n == 0)
						return *this;

					auto e = WSAGetLastError();
					switch (e) {
					case WSAEISCONN:
						return *this;
					case WSAEALREADY:
					case WSAEINPROGRESS:
					case WSAEWOULDBLOCK:
					case WSAEINTR: {
						auto sel = new_shared<select_t>();
						sel->push(this);
						sel->wait(timeout - (this_time - begin_time));
						break;
					}
					default: {
						this->close();
						_sock = (int)::socket(AF_INET, SOCK_STREAM, 0);
						setnonblock();
						this_thread::sleep_for(std::min(milliseconds(100), duration_cast<milliseconds>(timeout - (this_time - begin_time))));
						break;
					}
					}
				}
			}
			return *this;
		}


		socket_t& setblock(bool f) {
#ifdef this_platform_windows
			unsigned long ul = (f ? 0 : 1);
			::ioctlsocket(_sock, FIONBIO, (unsigned long *)&ul);
			_isblocked = f;
#endif
			return *this;
		}

		int getsockerror() {
			int err = 0;
#if defined(this_platform_windows)
			int len = sizeof(err);
#else
			socklen_t len = sizeof(err);
#endif
			::getsockopt(_sock, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
			return err;
		}

		bool			_isblocked = true;
		int				_sock = -1;
		uint16_t		_port = 0;
		string			_ip;
	};


	class listener_t {
	public:
		listener_t(uint16_t port) {
			_port = port;
			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = INADDR_ANY;
			_sock = (int)::socket(AF_INET, SOCK_STREAM, 0);
			unsigned long ul = 1;
			::ioctlsocket(_sock, FIONBIO, (unsigned long *)&ul);
			::bind(_sock, (sockaddr*)&addr, sizeof(addr));
			::listen(_sock, 1024);
		}
		~listener_t() {
			::closesocket(_sock);
		}

		shared_ptr<socket_t> accept(high_resolution_clock::duration timeout)
		{
			for (auto this_time = high_resolution_clock::now(), begin_time = this_time;
				this_time - begin_time <= timeout;
				this_time = high_resolution_clock::now()) 
			{	
				sockaddr_in addr;
				socklen_t	addrlen = sizeof(addr);
				auto n = (int)::accept(_sock, (sockaddr*)&addr, &addrlen);
				if (n >= 0) {
					auto sock = new_shared<socket_t>(n);
					return sock;
				}

				auto sel = new_shared<select_t>();
				sel->push(this);
				sel->wait(timeout - (this_time - begin_time));
			}
			return nullptr;
		}

		int get_fd() {
			return _sock;
		}

	private:
		int		 _sock = -1;
		uint16_t _port = 0;
	};



	class thread_pool_t {
	public:
		thread_pool_t(size_t count = thread::hardware_concurrency()) {
			for (size_t x = 0; x < count; ++x) {
				_threads.emplace_back([this]() {
					while (!_stop) {
						function<void()> job;
						if_lock(_jobs._mtx) {
							if (_jobs._hold.empty()) {
								_jobs._cond.wait_for(__ul,seconds(1), [this]() {
									return !_jobs._hold.empty() || _stop;
								});
								continue;
							}
							job.swap(_jobs._hold.front());
							_jobs._hold.pop_front();
						}
						if (job) {
							try {
								job();
							}
							catch (...) {

							}
						}
					}
				});
			}
		}
		~thread_pool_t() {
			_stop = true;
			_jobs._cond.notify_all();
			for (auto& t : _threads) {
				if (t.joinable())t.join();
			}
		}

		template<class ft, class... argst>
		auto push_front( ft&& f, argst&&... args )
		-> future<typename result_of<ft(argst...)>::type> {
			return move(push(true, forward<ft>(f), forward<argst>(args)...));
		}
		template<class ft, class... argst>
		auto push_back(ft&& f, argst&&... args)
			-> future<typename result_of<ft(argst...)>::type> {
			return move(push(false, forward<ft>(f), forward<argst>(args)...));
		}
	private:
		template<class ft, class... argst>
		auto push(bool putfront, ft&& f, argst&&... args)
			-> future<typename result_of<ft(argst...)>::type> {
			using return_t = typename result_of<ft(argst...)>::type;
			auto job = make_shared<packaged_task<return_t()>>(
				bind(forward<ft>(f), forward<argst>(args)...));
			auto ret = job->get_future();
			if_lock(_jobs._mtx) {
				if(putfront)
					_jobs._hold.emplace_front([job]() { (*job)(); });
				else
					_jobs._hold.emplace_back([job]() { (*job)(); });
			}
			_jobs._cond.notify_one();
			return move(ret);
		}

		bool								_stop = false;
		shared_t<list<function<void()>>>	_jobs;
		list<thread>						_threads;
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
		:_format(fmt){

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
			:_head(format), _filename(filename){
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
			fstream fs(_filename, ios::out|ios::binary);
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



//////////////////////////////////////////////////////////////  windows
#ifdef this_platform_windows

#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")

	namespace this_coroutine	{
		LPVOID get_id() {
			return ::GetCurrentFiber();
		}
	}

	template<class type>
	class coroutine_t {
		friend class coroutine_pool_t;
	public:
		template<class functype>
		coroutine_t(functype&& fn)
			:_callee_fn(std::forward<functype>(fn)) {
			this->_caller = ::ConvertThreadToFiberEx(this, FIBER_FLAG_FLOAT_SWITCH);
			if (!this->_caller) {
				this->_caller_is_callee = true;
				this->_caller = this_coroutine::get_id();
			}
				
			this->_callee = CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, &coroutine_t::callee_proc, this);
			::SwitchToFiber(this->_callee);
		}

		~coroutine_t() {
			clean_callee();
		}

		operator bool() {
			return _callee != nullptr;
		}

		type yield(const type& x) {
			_yield_type = x;
			switch_to_caller();
			return move(_resume_type);
		}

		type yield(type&& x) {
			_yield_type = move(x);
			switch_to_caller();
			return move(_resume_type);
		}


		coroutine_t<type>& resume(const type& x) {
			_resume_type = x;
			switch_to_callee();
			return *this;
		}

		coroutine_t<type>& resume(type&& x) {
			_resume_type = move(x);
			switch_to_callee();
			return *this;
		}

		type get() {
			return move(_yield_type);
		}

	private:

		void switch_to_callee() {
			if (this->_callee && this->_callee != this_coroutine::get_id()) {
				::SwitchToFiber(this->_callee);
			}
		}

		void switch_to_caller() {
			if(this->_caller && this->_caller != this_coroutine::get_id())
				::SwitchToFiber(this->_caller);
		}

		void clean_callee() {
			switch_to_caller();
			if (!_callee_quit) {
				_callee_quit = true;
				if (_callee) {
					::DeleteFiber(_callee);
					_callee = nullptr;
				}
			}
			if (!this->_caller_is_callee)
				::ConvertFiberToThread();
		}

		static void CALLBACK callee_proc(_In_ PVOID lpParameter) {
			auto co = (coroutine_t*)lpParameter;
			if (this_coroutine::get_id() == co->_callee) {
				co->_callee_fn(*co);
				co->clean_callee();
			}
		}

		type										_resume_type;
		type										_yield_type;
		bool										_caller_is_callee = false;
		bool										_callee_quit = false;
		LPVOID										_callee = nullptr;
		LPVOID										_caller = nullptr;
		function<void(coroutine_t<type>&)>			_callee_fn;
	};


#endif

}
using namespace this_space;
