
#include "myspace/strings/strings.h"

my_space_begin


string Strings::tolower(const string& src)
{
	string result;

	result.reserve(src.size());

	for (auto& c : src)
	{
		result.push_back(std::tolower(c));
	}
	return move(result);
}

string Strings::toupper(const string& src)
{
	string result;

	result.reserve(src.size());

	for (auto& c : src)
	{
		result.push_back(std::toupper(c));
	}
	return move(result);
}

list<string> Strings::lsplit(string& src, const string& tokens)
{
	list<string> ret;

	do
	{
		auto pos = src.find_first_of(tokens);

		if (pos == src.npos)
			break;

		ret.emplace_back(move(src.substr(0, pos)));

		src.erase(0, pos + 1);

	} while (true);

	return move(ret);
}

string Strings::strip(const string& src, const string& token)
{
	if (src.empty())
		return src;

	if (token.empty())
	{
		size_t first = 0;

		for (auto c : src)
		{
			if (std::isblank(c) || std::iscntrl(c))
			{
				first++;
				continue;
			}
			break;
		}

		size_t last = src.size();

		for (auto itr = src.rbegin(); itr != src.rend(); ++itr)
		{
			if (std::isblank(*itr) || std::iscntrl(*itr))
			{
				last--;
				continue;
			}
			break;
		}
		return move(src.substr(first, last - first));
	}

	auto beginpos = src.find_first_not_of(token);

	auto endpos = src.find_last_not_of(token);

	return move(src.substr(beginpos, endpos - beginpos));
}

deque<string> Strings::split(const string& src, const string& tokens)
{
	deque<string> ret;

	string tmp = src;

	for (size_t pos = src.find_first_of(tokens), last_pos = 0;
		last_pos != string::npos;
		last_pos = pos, pos = src.find_first_of(tokens, last_pos + tokens.size()))
	{
		ret.emplace_back(move(src.substr((last_pos == 0 ? 0 : last_pos + tokens.size()), pos)));
	}

	return move(ret);
}

deque<string> Strings::split(const string& src, char delm)
{
	return move(split(src, string(1, delm)));
}

deque<string> Strings::split(const char* src, char delm)
{
	return move(split(string(src), string(1, delm)));
}

string StringStream::str()
{
	return move(_ss.str());
}

my_space_end


