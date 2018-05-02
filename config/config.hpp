

#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/strings/strings.hpp"

MYSPACE_BEGIN

class Config {
public:
  Config(const std::string &path);

  template <class X = std::string>
  X get(const std::string &section, const std::string &key,
        const X &defaut) const;

  template <class X = std::string>
  X get(const std::string &section, const std::string &key) const;

private:
  std::unordered_map<std::string, std::unordered_map<std::string, std::string> >
  dict_;
};

inline Config::Config(const std::string &path) {
  bool multiline = false;
  std::string line, section, key, value;
  for (std::ifstream fs(path); getline(fs, line);) {
    line = Strings::stripOf(line);
    if (line.empty())
      continue;
    if (line[0] == '#' || line[0] == ';')
      continue;
    if (line[0] == '[') {
      section = line.substr(1, line.find_last_not_of(']'));
    } else {
      if (!multiline) {
        if (line.back() != '\\') {
          auto pos = line.find_first_of('=');
          key = Strings::stripOf(line.substr(0, pos));
          value = Strings::stripOf(line.substr(pos + 1));
          dict_[section][key] = value;
          value.clear();
        } else {
          line.pop_back();
          line = Strings::stripOf(line);
          auto pos = line.find_first_of('=');
          key = Strings::stripOf(line.substr(0, pos));
          value = Strings::stripOf(line.substr(pos + 1));
          multiline = true;
        }
      } else {
        if (line.back() != '\\') {
          multiline = false;
          value += line;
          dict_[section][key] = value;
          value.clear();
        } else {
          line.pop_back();
          value += Strings::stripOf(line);
        }
      }
    }
  }
  if (!value.empty())
    dict_[section][key] = value;
}

template <class X>
inline X Config::get(const std::string &section, const std::string &key,
                     const X &defaut) const {
  auto itr = dict_.find(section);
  if (itr == dict_.end())
    return defaut;
  auto iitr = itr->second.find(key);
  if (iitr == itr->second.end())
    return defaut;
  return StringStream(iitr->second);
}

template <class X>
inline X Config::get(const std::string &section, const std::string &key) const {
  auto itr = dict_.find(section);
  if (itr == dict_.end())
    MYSPACE_THROW("section (", section, ") not found");
  auto iitr = itr->second.find(key);
  if (iitr == itr->second.end())
    MYSPACE_THROW("key (", key, ") not found");
  return StringStream(iitr->second);
}

MYSPACE_END
