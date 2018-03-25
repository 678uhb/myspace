
#pragma once

#include "myspace/config.hpp"
#include "myspace/strings/strings.hpp"

MYSPACE_BEGIN


namespace
{
	void _dump(string& s, const exception& e)
	{
		try
		{
			rethrow_if_nested(e);
		}
		catch (const exception& x)
		{
			_dump(s, x);
		}
		catch (...)
		{

		}

		if (!s.empty())
		{
			s.append(1, '\n');
		}
		s.append(e.what());
	}

	void _dump(string& s)
	{
		try
		{
			auto e = current_exception();

			if (e)
			{
				rethrow_exception(e);
			}
		}
		catch (const exception &e)
		{
			_dump(s, e);
		}
		catch (...)
		{

		}
	};
}

class Exception : public exception 
{
public:

	template<class... Types>
	Exception(const char* file, int line, Types&&... types)
	{
		StringStream ss;
		ss << file << ":" << line << " ";
		ss.put(forward<Types>(types)...);
		_desc = move(ss.str());
	}

	template<class... Types>
	static void Throw(const char* file, int line, Types&&... types)
	{
		Exception(file, line, forward<Types>(types)...).Throw();
	}


	void Throw()
	{
		Throw(*this);
	}


	static void Throw(exception& e)
	{
		if (current_exception())
		{
			throw_with_nested(e);
		}
		else
		{
			throw move(e);
		}
	}

	static string dump()
	{
		string s;

		_dump(s);

		return move(s);
	}

	const char * what() const noexcept
	{
		return _desc.c_str();
	}

private:
	string	_desc;
};


#define THROW(...) Exception::Throw(__FILE__, __LINE__, ##__VA_ARGS__);

#define IF_THROW(x) if((x)) THROW(#x);


MYSPACE_END
