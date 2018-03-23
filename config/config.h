

#pragma once

#include "myspace/config.h"
#include "myspace/strings/strings.h"
#include "myspace/exception/exception.h"

myspace_begin

class Config
{
public:

	Config(const string& path);


	template<class type_t = string>
	type_t get(const string& section, const string& key, const type_t& defaut) 
	{
		auto itr = _dict.find(section);

		if (itr == _dict.end())
		{
			return defaut;
		}

		auto iitr = itr->second.find(key);

		if (iitr == itr->second.end())
		{
			return defaut;
		}

		return StringStream(iitr->second);
	}

	template<class type_t = string>
	type_t get(const string& section, const string& key)
	{
		auto itr = _dict.find(section);

		if (itr == _dict.end())
		{
			ethrow("section (", section, ") not found");
		}

		auto iitr = itr->second.find(key);

		if (iitr == itr->second.end())
		{
			ethrow("key (", key, ") not found");
		}

		return StringStream(iitr->second);
	}

private:
	unordered_map<string, unordered_map<string, string>> _dict;
};
myspace_end
