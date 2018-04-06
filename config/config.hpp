

#pragma once

#include "myspace/_/include.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/strings/strings.hpp"

MYSPACE_BEGIN

class Config {
public:
  Config(const string &path);

  template <class X = string>
  X get(const string &section, const string &key, const X &defaut) const;

  template <class X = string>
  X get(const string &section, const string &key) const;

private:
  unordered_map<string, unordered_map<string, string>> dict_;
};

inline Config::Config(const string &path) {
  bool multiline = false;
  string line, section, key, value;
  for (ifstream fs(path); getline(fs, line);) {
    line = Strings::strip(line);
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
          key = Strings::strip(line.substr(0, pos));
          value = Strings::strip(line.substr(pos + 1));
          dict_[section][key] = value;
          value.clear();
        } else {
          line.pop_back();
          line = Strings::strip(line);
          auto pos = line.find_first_of('=');
          key = Strings::strip(line.substr(0, pos));
          value = Strings::strip(line.substr(pos + 1));
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
          value += Strings::strip(line);
        }
      }
    }
  }
  if (!value.empty())
    dict_[section][key] = value;
}

template <class X>
inline X Config::get(const string &section, const string &key,
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
inline X Config::get(const string &section, const string &key) const {
  auto itr = dict_.find(section);
  if (itr == dict_.end())
    MYSPACE_THROW("section (", section, ") not found");
  auto iitr = itr->second.find(key);
  if (iitr == itr->second.end())
    MYSPACE_THROW("key (", key, ") not found");
  return StringStream(iitr->second);
}

MYSPACE_END
