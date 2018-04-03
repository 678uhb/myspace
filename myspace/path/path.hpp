

#pragma once


#include "myspace/myspace_include.h"

MYSPACE_BEGIN

class Path
{
public:

	static string basename(const string& path)
	{
		auto pos = path.find_last_of("\\/");

		if (pos != string::npos)
			return move(path.substr(pos + 1));

		return move(path);
	}

	static string basenameNoext(const string& path)
	{
		return move(splitext(basename(path))[0]);
	}


	static deque<string> splitext(const string& path)
	{
		auto pos = path.find_last_of('.');

		if (pos == path.npos)
		{
			return {path, ""};
		}

		return{ path.substr(0, pos), path.substr(pos + 1) };
	}

	static string dirname(const string& path)
	{
		auto pos = path.find_last_of("/\\");
		
		if (pos == path.npos)
			return path;

		return move(path.substr(0, pos));
	}
};

MYSPACE_END


