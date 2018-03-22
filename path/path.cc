

#include "myspace/path/path.h"

myspace_begin

string Path::basename(const string& path)
{
	auto pos = path.find_last_of("\\/");

	if (pos != string::npos)
		return move(path.substr(pos + 1));

	return move(path);
}


myspace_end
