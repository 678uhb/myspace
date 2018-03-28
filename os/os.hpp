
#pragma once

#include "myspace/myspace_include.h"

myspace_begin

class OS
{
public:

	static size_t filesize(const string& path)
	{
		ifstream ifs(path, ios::in | ios::binary);

		if (!ifs.is_open())
			return 0;

		ifs.seekg(0, ios::end);

		return ifs.tellg();
	}
};

myspace_end
