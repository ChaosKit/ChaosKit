#include "ManagedDocument.h"
#include "DocumentDeleter.h"

namespace chaoskit::core {

void detail::Deleter::operator()(core::Document* doc) {
  if (doc == nullptr) return;
  DocumentDeleter().visit(*doc);
  delete doc;
}

ManagedDocument makeManagedDocument() {
  return ManagedDocument(new core::Document(), detail::Deleter());
}

}  // namespace chaoskit::core
