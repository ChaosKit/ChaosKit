#ifndef CHAOSKIT_FLAME_MANAGEDDOCUMENT_H
#define CHAOSKIT_FLAME_MANAGEDDOCUMENT_H

#include <memory>
#include "Document.h"

namespace chaoskit::flame {
namespace detail {

class Deleter {
 public:
  void operator()(Document* doc);
};

}  // namespace detail

using ManagedDocument = std::unique_ptr<Document, detail::Deleter>;
ManagedDocument makeManagedDocument();

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_MANAGEDDOCUMENT_H
