

#pragma once


#include "myspace/config.hpp"

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

};

MYSPACE_END


