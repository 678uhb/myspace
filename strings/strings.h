
#pragma once

#include "myspace/config.h"

my_space_begin

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

class StringStream
{
public:

	template<class Type>
	StringStream& operator << (const Type& x)
	{
		_ss << x;
		return *this;
	}

	template<class Type>
	operator Type()
	{
		Type x;
		_ss >> x;
		return move(x);
	}

	string str();

private:
	stringstream _ss;
};

my_space_end

