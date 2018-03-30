

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
		bool multiline = false;
		
		string line;
		
		string section;
		
		string key, value;

		for (ifstream fs(path); getline(fs, line); )
		{
			line = Strings::strip(line);

			if (line.empty()) continue;

			if(line[0] == '#' || line[0] == ';') continue;

			if (line[0] == '[')
			{
				section = line.substr(1, line.find_last_not_of(']'));
			}
			else
			{
				if(!multiline)
				{
					if (line.back() != '\\')
					{
						auto pos = line.find_first_of('=');

						key = Strings::strip(line.substr(0, pos));

						value = Strings::strip(line.substr(pos + 1));

						_dict[section][key] = value;
						value.clear();
					}
					else
					{
						line.pop_back();
						line = Strings::strip(line);

						auto pos = line.find_first_of('=');

						key = Strings::strip(line.substr(0, pos));

						value = Strings::strip(line.substr(pos + 1));

						multiline = true;
					}
				}
				else
				{
					if (line.back() != '\\')
					{
						multiline = false;

						value += line;

						_dict[section][key] = value;
						value.clear();
					}
					else
					{
						line.pop_back();
						value += Strings::strip(line);
					}
				}
			}
		}

		if(!value.empty()) _dict[section][key] = value;
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
