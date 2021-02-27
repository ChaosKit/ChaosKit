#ifndef CHAOSKIT_UI_STORAGE_H
#define CHAOSKIT_UI_STORAGE_H

#include <stdexcept>
#include "flame/Document.h"
#include "flame/DocumentVisitor.h"

namespace chaoskit::ui {

class StorageError : public std::runtime_error {
 public:
  explicit StorageError(const char* what) : std::runtime_error(what) {}
  explicit StorageError(const std::string& what) : std::runtime_error(what) {}
};

void loadFromFile(const std::string& path, flame::Document* document);
void saveToFile(const std::string& path, const flame::Document& document);

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_STORAGE_H
