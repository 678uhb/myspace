#pragma once

#include "myspace/config/config.h"
#include "myspace/strings/strings.h"

myspace_begin

Config::Config(const string& path)
{
	string line;
	string section = "this";
	for (ifstream fs(path); getline(fs, line); ) 
	{
		if (Strings::strip(line).empty())
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

myspace_end
