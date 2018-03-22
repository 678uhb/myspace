
#pragma once

#include "myspace/config.h"

myspace_begin

class Strings
{
public:

	static string tolower(const string& src);

	static string toupper(const string& src);

	static list<string> lsplit(string& src, const string& tokens);

	static string strip(const string& src, const string& token = "");

	static deque<string> split(const string& src, const string& tokens);

	static deque<string> split(const string& src, char delm);

	static deque<string> split(const char* src, char delm);
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

	string str();

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

