
#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

template <class Type, class... Arguments>
inline typename std::enable_if<!std::is_array<Type>::value,
                               std::unique_ptr<Type>>::type
newUnique(Arguments &&... args) {
  return std::unique_ptr<Type>(new Type(std::forward<Arguments>(args)...));
}

template <class Type>
inline typename std::enable_if<std::is_array<Type>::value &&
                                   std::extent<Type>::value == 0,
                               std::unique_ptr<Type>>::type
newUnique(size_t count) {
  typedef typename std::remove_extent<Type>::type T;
  return std::unique_ptr<Type>(new T[count]());
}

template <class Type, class... Arguments>
inline std::shared_ptr<Type> newShared(Arguments &&... args) {
  return std::move(std::make_shared<Type>(std::forward<Arguments>(args)...));
}

MYSPACE_END
