

#pragma once

#include "myspace/config.h"

my_space_begin

class Config 
{
public:

	Config(const string& path);

	template<class type_t = string>
	type_t get(const string& sk) 
	{
		class find_t {};
		string s, k, v;
		try 
		{
			auto pos = sk.find_first_of('.');
			if (pos != string::npos) 
			{
				s = move(sk.substr(0, pos));

				k = move(sk.substr(pos + 1));

				auto itr = _dict.find(s);

				if (itr != _dict.end()) 
				{
					auto iitr = itr->second.find(k);
					if (iitr != itr->second.end()) 
					{
						v = iitr->second;
						throw find_t();
					}
				}
			}
			auto itr = _dict["this"].find(sk);
			if (itr != _dict["this"].end()) 
			{
				v = itr->second;
				throw find_t();
			}
		}
		catch (find_t&) 
		{
			stringstream ss;
			ss << v;
			type_t x;
			ss >> x;
			return move(x);
		}
		throw_runtimerror("%s configure not found", sk.c_str());
		return type_t();
	}


	template<class type_t = string>
	type_t get(const string& sk, const type_t& def) 
	{
		try 
		{
			return get<type_t>(sk);
		}
		catch (...)
		{
			return move(type_t(def));
		}
	}
private:
	unordered_map<string, unordered_map<string, string>> _dict;
};
my_space_end