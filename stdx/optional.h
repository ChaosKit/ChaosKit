#ifndef CHAOSKIT_OPTIONAL_H
#define CHAOSKIT_OPTIONAL_H

#if __has_include(<optional>)

#include <optional>
namespace stdx {
  using namespace ::std;
}

#elif __has_include(<experimental/optional>)

#include <experimental/optional>
namespace stdx {
using namespace ::std;
using namespace ::std::experimental;
}

#else
#error C++17 Optionals are required.
#endif

#endif //CHAOSKIT_OPTIONAL_H
