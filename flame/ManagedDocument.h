#ifndef CHAOSKIT_CORE_MANAGEDDOCUMENT_H
#define CHAOSKIT_CORE_MANAGEDDOCUMENT_H

#include <memory>
#include "Document.h"

namespace chaoskit::core {

namespace detail {

class Deleter {
 public:
  void operator()(core::Document* doc);
};

}  // namespace detail

using ManagedDocument = std::unique_ptr<Document, detail::Deleter>;
ManagedDocument makeManagedDocument();

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_MANAGEDDOCUMENT_H
