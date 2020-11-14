#ifndef CHAOSKIT_IO_IO_H
#define CHAOSKIT_IO_IO_H

#include <stdexcept>
#include "core/DocumentVisitor.h"
#include "flame/Document.h"

namespace chaoskit::io {

class Error : public std::runtime_error {
 public:
  explicit Error(const char* what) : std::runtime_error(what) {}
  explicit Error(const std::string& what) : std::runtime_error(what) {}
};

void loadFromFile(const std::string& path, core::Document* document);
void saveToFile(const std::string& path, const core::Document& document);

}  // namespace chaoskit::io

#endif  // CHAOSKIT_IO_IO_H
