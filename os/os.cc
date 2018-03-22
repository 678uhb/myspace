

#include "myspace/os/os.h"

myspace_begin

size_t OS::filesize(const string& path)
{
	ifstream ifs(path, ios::in | ios::binary);

	if (!ifs.is_open())
		return 0;

	ifs.seekg(0, ios::end);

	return ifs.tellg();
}

myspace_end
