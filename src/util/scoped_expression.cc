#pragma once

#include <functional>

namespace util {

struct ScopedExpression {
  ScopedExpression(const std::function<void()>& func) :
    func(func) {}
  ~ScopedExpression() { func(); }

  std::function<void()> func;
};

}
