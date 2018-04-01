
#pragma once

#include "myspace/myspace_include.h"
#include "myspace/memory/memory.hpp"
#include "myspace/path/path.hpp"

MYSPACE_BEGIN

class Process
{
public:
	static string getMyFullName()
	{
		const size_t maxPath = 4096;

		auto name = new_unique<char[]>(maxPath);
		int n = 0;
#if defined(myspace_windows)
		n = GetModuleFileName(NULL, name.get(), maxPath);
#elif defined(myspace_linux)
		n = readlink("/proc/self/exe", name.get(), maxPath);
#endif
		if (n <= 0)
			return "";
		return move(string(name.get(), n));
	}

	static string getMyName()
	{
		return move(Path::basename(getMyFullName()));
	}

	static string getMyNameNoExt()
	{
		return move(Path::basenameNoext(getMyFullName()));
	}

	static string cwd(const string& path = "")
	{
		if (path.empty())
		{
			const size_t maxPath = 4096;

			auto name = new_unique<char[]>(maxPath);

#if defined(myspace_windows)

			auto n = GetCurrentDirectory(maxPath, name.get());

			if (n <= 0) return "";

			return move(string(name.get(), n));
#else
			auto n = readlink("/proc/self/cwd", name.get(), maxPath);

			if (n <= 0) return "";

			return move(string(name.get(), n));
#endif
		}
#if defined(myspace_windows)
		SetCurrentDirectory(path.c_str());
#else
		chdir(path.c_str());
#endif
		return cwd();
	}
};

MYSPACE_END
