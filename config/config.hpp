

#pragma once

#include "myspace/myspace_include.h"
#include "myspace/strings/strings.hpp"
#include "myspace/exception/exception.hpp"

myspace_begin

class Config
{
public:


	Config(const string& path)
	{
		string line;
		string section = "this";
		for (ifstream fs(path); getline(fs, line); )
		{
			line = Strings::strip(line);

			if (line.empty())
				continue;

			if (line[0] == '[')
			{
				section = line.substr(1, line.find_last_not_of(']'));
			}
			else
			{
				auto pos = line.find_first_of('=');
				string key = move(line.substr(0, pos));
				string value = move(line.substr(pos + 1));
				_dict[section][move(key)] = value;
			}
		}
	}


	template<class type_t = string>
	type_t get(const string& section, const string& key, const type_t& defaut) const
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
	type_t get(const string& section, const string& key) const
	{
		auto itr = _dict.find(section);

		if (itr == _dict.end())
		{
			my_throw("section (", section, ") not found");
		}

		auto iitr = itr->second.find(key);

		if (iitr == itr->second.end())
		{
			my_throw("key (", key, ") not found");
		}

		return StringStream(iitr->second);
	}

private:
	unordered_map<string, unordered_map<string, string>> _dict;
};
myspace_end
