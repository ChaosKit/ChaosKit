#include "ManagedDocument.h"
#include "Document.h"
#include "DocumentDeleter.h"

namespace chaoskit::flame {

void detail::Deleter::operator()(Document* doc) {
  if (doc == nullptr) return;
  chaoskit::flame::DocumentDeleter().visit(*doc);
  delete doc;
}

ManagedDocument makeManagedDocument() {
  return ManagedDocument(new Document(), detail::Deleter());
}

}  // namespace chaoskit::flame
