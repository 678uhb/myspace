
#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

namespace Strings {

inline std::string tolower(const std::string &src);

inline std::string toupper(const std::string &src);

inline std::string strip(const std::string &src, const std::string &delme = "");

template <class ReturnType, class SourceType, class DelmeType>
inline ReturnType splitOf(const SourceType &, const DelmeType &);

template <class ReturnType, class SourceType, class X>
inline ReturnType splitOf(const SourceType &, std::initializer_list<X> init);

template <class ReturnType = std::deque<std::string>>
inline ReturnType splitOf(const std::string &src, const std::string &delme);

template <class ReturnType = std::deque<std::string>>
inline ReturnType splitOf(const std::string &src, const char &delme);

template <class Iteraterable>
inline std::string join(const Iteraterable &x, const std::string &join_token);

template <class Iteraterable>
inline std::string join(const Iteraterable &x, const char);
}; // namespace Strings

inline std::string Strings::tolower(const std::string &src) {
  std::string result;

  result.reserve(src.size());

  for (auto &c : src) {
    result.push_back(std::tolower(c));
  }
  return result;
}

inline std::string Strings::toupper(const std::string &src) {
  std::string result;

  result.reserve(src.size());

  for (auto &c : src) {
    result.push_back(std::toupper(c));
  }
  return result;
}

inline std::string Strings::strip(const std::string &src,
                                  const std::string &delme) {
  if (src.empty())
    return src;

  if (delme.empty()) {
    size_t first = 0;

    for (auto c : src) {
      if (std::isblank(c) || std::iscntrl(c)) {
        first++;
        continue;
      }
      break;
    }

    size_t last = src.size();

    for (auto itr = src.rbegin(); itr != src.rend(); ++itr) {
      if (std::isblank(*itr) || std::iscntrl(*itr)) {
        last--;
        continue;
      }
      break;
    }
    return src.substr(first, last - first);
  }

  auto beginpos = src.find_first_not_of(delme);

  auto endpos = src.find_last_not_of(delme);

  return src.substr(beginpos, endpos - beginpos);
}

template <class ReturnType, class SourceType, class DelmeType>
inline ReturnType Strings::splitOf(const SourceType &src,
                                   const DelmeType &delme) {
  ReturnType ret;
  SourceType one;
  for (auto &s : src) {
    if (none_of(
            begin(delme), end(delme),
            [&s](const typename DelmeType::value_type &d) { return s == d; })) {
      fill_n(inserter(one, end(one)), 1, s);
    } else if (!one.empty()) {
      fill_n(inserter(ret, end(ret)), 1, one);
      one.clear();
    }
  }
  if (!one.empty()) {
    fill_n(inserter(ret, end(ret)), 1, one);
  }
  return ret;
}

template <class ReturnType, class SourceType, class X>
inline ReturnType Strings::splitOf(const SourceType &src,
                                   std::initializer_list<X> init) {
  return splitOf<ReturnType, SourceType, SourceType>(src, SourceType(init));
}

template <class ReturnType>
inline ReturnType Strings::splitOf(const std::string &src,
                                   const std::string &delme) {
  return splitOf<ReturnType, std::string, std::string>(src, delme);
}

template <class ReturnType>
inline ReturnType Strings::splitOf(const std::string &src, const char &delme) {
  return splitOf<ReturnType, std::string, std::string>(src, std::string{delme});
}

template <class Iteraterable>
inline std::string Strings::join(const Iteraterable &x,
                                 const std::string &join_token) {

	bool first = true;
	std::string result;
	for (auto itr = begin(x); itr != end(x); itr = next(itr)) {
		if (!first) {
			result.append(join_token);
		}
		first = false;
		result.append(*itr);
	}
	return std::move(result);
}

template <class Iteraterable>
inline std::string Strings::join(const Iteraterable &x, char join_token) {
  return Strings::join<Iteraterable>(x, string{join_token});
}

MYSPACE_END
