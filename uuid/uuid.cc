
#include "myspace/uuid/uuid.h"

my_space_begin

string UUID::gen()
{
#ifdef this_platform_windows
	GUID guid;
	string uuid;
	::CoCreateGuid(&guid);
	{
		stringstream ss;
		ss << hex << setw(sizeof(guid.Data1) << 1) << setfill('0') << guid.Data1;
		uuid.append(move(ss.str()));
		uuid.append(1, '-');
	}
	{
		stringstream ss;
		ss << hex << setw(sizeof(guid.Data2) << 1) << setfill('0') << guid.Data2;
		uuid.append(move(ss.str()));
		uuid.append(1, '-');
	}
	{
		stringstream ss;
		ss << hex << setw(sizeof(guid.Data3) << 1) << setfill('0') << guid.Data3;
		uuid.append(move(ss.str()));
		uuid.append(1, '-');
	}
	for (auto i = 0; i < sizeof(guid.Data4); ++i)
	{
		stringstream ss;
		ss << hex << setw(2) << setfill('0') << (int)guid.Data4[i];
		uuid.append(move(ss.str()));
	}
	return move(uuid);
#endif
#ifdef this_platform_linux
	uuid_t uid;
	uuid_generate(uid);
	stringstream ss;
	for (size_t i = 0; i < sizeof(uid); ++i)
		ss << hex << setw(2) << setfill('0') << (int)((char*)&uid)[0];
	return move(ss.str());
#endif
}

my_space_end
