
#pragma once

#include "myspace/_/include.hpp"
#include "myspace/critical/critical.hpp"

MYSPACE_BEGIN

class CoroutinePool {
public:
  CoroutinePool(size_t size = 10);

  ~CoroutinePool();

  template <class Function, class Arguments>
  auto pushFront(Function &&f, Arguments &&... args)
      -> future<typename result_of<Function(Arguments...)>::type>;

  template <class Function, class... Arguments>
  auto pushBack(Function &&f, Arguments &&... args)
      -> future<typename result_of<Function(Arguments...)>::type>;
};

MYSPACE_END
