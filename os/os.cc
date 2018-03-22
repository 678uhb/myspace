

#include "myspace/os/os.h"

my_space_begin

size_t OS::filesize(const string& path)
{
	ifstream ifs(path, ios::in | ios::binary);

	if (!ifs.is_open())
		return 0;

	ifs.seekg(0, ios::end);

	return ifs.tellg();
}

my_space_end
