
#pragma once

#include "myspace/myspace_include.h"

myspace_begin

class Strings
{
public:


	static string tolower(const string& src)
	{
		string result;

		result.reserve(src.size());

		for (auto& c : src)
		{
			result.push_back(std::tolower(c));
		}
		return move(result);
	}



	static string toupper(const string& src)
	{
		string result;

		result.reserve(src.size());

		for (auto& c : src)
		{
			result.push_back(std::toupper(c));
		}
		return move(result);
	}



	static list<string> lsplit(string& src, const string& tokens)
	{
		list<string> ret;

		do
		{
			auto pos = src.find_first_of(tokens);

			if (pos == src.npos)
				break;

			ret.emplace_back(move(src.substr(0, pos)));

			src.erase(0, pos + 1);

		} while (true);

		return move(ret);
	}



	static string strip(const string& src, const string& token = "")
	{
		if (src.empty())
			return src;

		if (token.empty())
		{
			size_t first = 0;

			for (auto c : src)
			{
				if (std::isblank(c) || std::iscntrl(c))
				{
					first++;
					continue;
				}
				break;
			}

			size_t last = src.size();

			for (auto itr = src.rbegin(); itr != src.rend(); ++itr)
			{
				if (std::isblank(*itr) || std::iscntrl(*itr))
				{
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



	static deque<string> split(const string& src, const string& tokens)
	{
		deque<string> ret;

		string tmp = src;

		for (size_t pos = src.find_first_of(tokens), last_pos = 0;
			last_pos != string::npos;
			last_pos = pos, pos = src.find_first_of(tokens, last_pos + tokens.size()))
		{
			ret.emplace_back(move(src.substr((last_pos == 0 ? 0 : last_pos + tokens.size()), pos)));
		}

		return move(ret);
	}




	static deque<string> split(const string& src, char delm)
	{
		return move(split(src, string(1, delm)));
	}


	static deque<string> split(const char* src, char delm)
	{
		return move(split(string(src), string(1, delm)));
	}

};

class StringStream : public stringstream
{
public:

	template<class... Targs>
	StringStream(Targs&&... args)
	{
		put(forward<Targs>(args)...);
	}

	template<class Type>
	StringStream& operator << (const Type& x)
	{
		_ss << x;
		return *this;
	}

	template<class Type>
	StringStream& operator >> (Type& x)
	{
		_ss >> x;
		return *this;
	}

	template<class Type>
	operator Type ()
	{
		Type x;
		_ss >> x;
		return x;
	}


	string str()
	{
		return _ss.str();
	}

	template<class... Targs>
	StringStream& put(Targs&&... args)
	{
		_put(forward<Targs>(args)...);
		return *this;
	}

private:

	template<class T, class... Targs>
	StringStream& _put(T&& x, Targs&&... args)
	{
		_ss << x;
		return _put(forward<Targs>(args)...);
	}

	template<class T>
	StringStream& _put(T&& x)
	{
		_ss << x;
		return *this;
	}

	StringStream& _put()
	{
		return *this;
	}

private:
	stringstream _ss;
};

myspace_end

