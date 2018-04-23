#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/concurrency/deque.hpp"

MYSPACE_BEGIN

namespace concurrency {
class Factory {
public:
  template <class X>
  static std::shared_ptr<myspace::concurrency::Deque<X>>
  createDeque(size_t max_size = std::deque<X>().max_size());
};

template <class X>
inline std::shared_ptr<myspace::concurrency::Deque<X>>
Factory::createDeque(size_t max_size) {
  return std::shared_ptr<concurrency::Deque<X>>(
      new concurrency::Deque<X>(max_size));
}

} // namespace concurrency

MYSPACE_END
