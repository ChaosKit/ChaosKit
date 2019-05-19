#include <iostream>
#include <vector>
#include "core/models/ModelCollection.h"
#include "core/models/SystemModel.h"
#include "core/structures/System.h"

using namespace chaoskit::core;

int main(int argc, char** argv) {
  std::vector<std::shared_ptr<System>> systems{};
  ModelCollection<SystemModel> collection(systems);

  return 0;
}
