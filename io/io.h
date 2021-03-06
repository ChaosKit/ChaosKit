#ifndef CHAOSKIT_IO_IO_H
#define CHAOSKIT_IO_IO_H

#include <stdexcept>
#include "flame/Document.h"
#include "flame/DocumentVisitor.h"

namespace chaoskit::io {

class Error : public std::runtime_error {
 public:
  explicit Error(const char* what) : std::runtime_error(what) {}
  explicit Error(const std::string& what) : std::runtime_error(what) {}
};

void loadFromFile(const std::string& path, flame::Document* document);
void saveToFile(const std::string& path, const flame::Document& document);

}  // namespace chaoskit::io

#endif  // CHAOSKIT_IO_IO_H
