
#pragma once

#include "myspace/config.hpp"
#include "myspace/error/error.hpp"
#include "myspace/detector/detector.hpp"
#include "myspace/socket/socketopt.hpp"

MYSPACE_BEGIN

class Socket
{
public:
	Socket()
	{
	}

	Socket(int sock)
	{
		_sock = sock;
	}

	Socket(const string& addr, high_resolution_clock::duration timeout = hours(999))
	{
		setAddr(addr.c_str()).connect(timeout);
	}

	Socket(const string& addr, uint16_t port, high_resolution_clock::duration timeout = hours(999))
	{
		setAddr(addr.c_str()).setPort(port).connect(timeout);
	}

	~Socket()
	{
		close();
	}

	size_t send(const string& data, high_resolution_clock::duration timeout)
	{
		size_t sendn = 0;

		for (auto this_time = high_resolution_clock::now(), begin_time = this_time;
			sendn < data.size() && this_time - begin_time <= timeout
			; this_time = high_resolution_clock::now())
		{
			auto n = ::send(_sock, data.c_str() + sendn, int(data.size() - sendn), 0);

			if (n > 0)
				sendn += n;

			else if (n == 0)
				break;

			else
			{
				auto e = Error::lastNetError();

				if (e == Error::wouldblock || e == Error::intr || e == Error::inprogress)
				{
					auto sel = new_shared<Detector>();
					sel->add(this, DetectType::WRITE);
					sel->wait(timeout - (this_time - begin_time));
					continue;
				}

				break;
			}
		}
		return sendn;
	}

	size_t send(const string & data)
	{
		size_t sendn = 0;

		while (sendn < data.size())
		{
			auto n = ::send(_sock, data.c_str() + sendn, int(data.size() - sendn), 0);

			if (n > 0)
				sendn += n;

			else if (n == 0)
				break;

			else
			{
				auto e = Error::lastNetError();

				if (e == Error::wouldblock || e == Error::intr || e == Error::inprogress)
				{
					continue;
				}

				break;
			}
		}
		return sendn;
	}

	string recv(high_resolution_clock::duration timeout)
	{
		string data;

		size_t buflen = 4096;

		auto buf = new_unique<char[]>(buflen);

		for (auto begin_time = high_resolution_clock::now(), this_time = begin_time;
			this_time - begin_time <= timeout;
			this_time = high_resolution_clock::now())
		{
			auto n = ::recv(_sock, buf.get(), (int)buflen, 0);

			if (n > 0)
				data.append(buf.get(), n);

			else if (n == 0)
				break;

			else
			{
				auto e = Error::lastNetError();

				if (e == Error::wouldblock || e == Error::intr || e == Error::inprogress)
				{
					auto sel = new_shared<Detector>();
					sel->add(this);
					sel->wait(timeout - (this_time - begin_time));
					continue;
				}

				break;
			}
		}
		return move(data);
	}

	string recv(size_t len, high_resolution_clock::duration timeout)
	{
		if (len == 0)
			return "";

		size_t recvn = 0;

		auto buf = new_unique<char[]>(len);

		for (auto this_time = high_resolution_clock::now(), begin_time = this_time;
			recvn < len && this_time - begin_time <= timeout;
			this_time = high_resolution_clock::now())
		{
			auto n = ::recv(_sock, buf.get() + recvn, int(len - recvn), 0);

			if (n > 0)
			{
				recvn += n;
			}

			else if (n == 0)
				break;

			else
			{
				auto e = Error::lastNetError();
				if (e == Error::wouldblock || e == Error::intr || e == Error::inprogress)
				{
					auto sel = new_shared<Detector>();
					sel->add(this);
					sel->wait(timeout - (this_time - begin_time));
					continue;
				}
				break;
			}
		}
		return move(string(buf.get(), recvn));
	}


	string recv(size_t len)
	{
		if (len == 0)
			return "";

		size_t recvn = 0;

		auto buf = new_unique<char[]>(len);

		while (recvn < len)
		{
			auto n = ::recv(_sock, buf.get() + recvn, int(len - recvn), 0);

			if (n > 0)
			{
				recvn += n;
			}

			else if (n == 0)
				break;

			else
			{
				auto e = Error::lastNetError();
				if (e == Error::wouldblock || e == Error::intr || e == Error::inprogress)
				{
					continue;
				}
				break;
			}
		}
		return move(string(buf.get(), recvn));
	}


	string recv_until(const string& delm, high_resolution_clock::duration timeout)
	{
		if (delm.empty())
			return move(recv(timeout));

		size_t recvn = delm.size();

		string ret;

		auto buf = new_unique<char[]>(recvn);

		for (auto this_time = system_clock::now(), begin_time = this_time;
			(ret.size() < delm.size() || ret.rfind(delm, ret.size() - delm.size()) == ret.npos)
			&& this_time - begin_time <= timeout;
			this_time = system_clock::now())
		{
			auto n = ::recv(_sock, buf.get(), int(recvn), 0);

			if (n == 0)
				break;

			if (n < 0)
			{
				auto e = Error::lastNetError();

				if (e == Error::wouldblock || e == Error::intr || e == Error::inprogress)
				{
					auto sel = new_shared<Detector>();
					sel->add(this);
					sel->wait(timeout - (this_time - begin_time));
					continue;
				}
				break;
			}

			ret.append(buf.get(), n);
			recvn = 0;
			for (size_t maxmatchn = std::min(delm.size(), ret.size()); maxmatchn; --maxmatchn)
			{
				if (ret.compare(ret.size() - maxmatchn, maxmatchn, delm, 0, maxmatchn) == 0)
				{
					recvn = delm.size() - maxmatchn;
					break;
				}
			}
			if (!recvn) recvn = delm.size();
		}
		return move(ret);
	}


	Socket& setBlock()
	{
		return setBlock(true);
	}

	Socket& setNonBlock()
	{
		return setBlock(false);
	}


	bool isBlocked() const
	{
		return _isblocked;
	}


	operator bool() 
	{
		return isConnected();
	}

	int getFd() const
	{
		return _sock;
	}


	bool isConnected() 
	{
		auto isblocked = _isblocked;
		DEFER(setBlock(isblocked));
	again:
		char c;
		setNonBlock();
		auto n = ::recv(_sock, &c, 1, MSG_PEEK);
		if (n == 0)
			return false;
		if (n > 0)
			return true;
		auto e = Error::lastNetError();

		switch (e) {
		case Error::wouldblock:
		case Error::inprogress:
			return true;
		case Error::intr:
			goto again;
		default:
			return false;
		}
	}

	void close()
	{
		if (_sock >= 0) {
			setBlock(true);
			Socket::close(_sock);
			_sock = -1;
		}
	}


	static void close(int sock)
	{
#ifdef MS_WINDOWS
		::closesocket(sock);
#else
		::close(sock);
#endif
	}

	bool operator==(const Socket & s) const
	{
		return _sock == s._sock;
	}


	operator int() const
	{
		return _sock;
	}

private:

	Socket& setPort(uint16_t port)
	{
		_port = port; return *this;
	}

	Socket& setAddr(const char* ipport)
	{
		auto tokens = Strings::split(ipport, ':');

		if (tokens.size() == 1)
		{
			_ip = Strings::strip(tokens[0]);
		}
		else if (tokens.size() >= 2)
		{
			_ip = Strings::strip(tokens[0]);
			_port = StringStream(Strings::strip(tokens[1]));
		}
		return *this;
	}


	Socket& connect(high_resolution_clock::duration timeout)
	{
		if (!_ip.empty() || _port != 0)
		{

			this->close();

			_sock = (int)::socket(AF_INET, SOCK_STREAM, 0);

			setNonBlock();

			for (auto this_time = high_resolution_clock::now(), begin_time = this_time;
				!isConnected() && this_time - begin_time <= timeout;
				this_time = high_resolution_clock::now())
			{

				sockaddr_in addr = { 0 };
				addr.sin_family = AF_INET;
				addr.sin_port = htons(_port);
				inet_pton(AF_INET, _ip.c_str(), &addr.sin_addr.s_addr);

				auto n = ::connect(_sock, (sockaddr*)&addr, sizeof(addr));

				if (n == 0)
					return *this;

				auto e = Error::lastNetError();

				switch (e)
				{
				case Error::isconn:
					return *this;
				case Error::already:
				case Error::inprogress:
				case Error::wouldblock:
				case Error::intr:
				{
					if (isConnected())
						return *this;

					auto sel = new_shared<Detector>();

					sel->add(this);

					sel->wait(timeout - (this_time - begin_time));
					break;
				}
				default:
				{
					this->close();
					_sock = (int)::socket(AF_INET, SOCK_STREAM, 0);
					setNonBlock();
					this_thread::sleep_for(std::min(milliseconds(100), duration_cast<milliseconds>(timeout - (this_time - begin_time))));
					break;
				}
				}
			}
		}
		return *this;
	}




	Socket& setBlock(bool f)
	{
		SocketOpt::setBlock(_sock, f);
		_isblocked = f;
		return *this;
	}

	int getSockError()
	{
		int err = 0;
#if defined(MS_WINDOWS)
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




MYSPACE_END
