#ifndef CHAOSKIT_STATE_STORE_H
#define CHAOSKIT_STATE_STORE_H

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <mapbox/variant.hpp>
#include "Id.h"

namespace chaoskit::state {

class IdTypeMismatchError : public std::logic_error {
 public:
  explicit IdTypeMismatchError(const char* what) : std::logic_error(what) {}
};

class MissingIdError : public std::logic_error {
 public:
  explicit MissingIdError(const char* what) : std::logic_error(what) {}
};

template <typename... Ts>
class Store {
  using Entity = mapbox::util::variant<Ts...>;

  struct TransactionData {
    std::unordered_set<Id> created;
    std::unordered_map<Id, Entity> updated;
    std::unordered_map<Id, Entity> removed;
  };

  std::unordered_map<Id, Entity> entities_;
  std::vector<uint32_t> counters_;
  std::unique_ptr<TransactionData> currentTransaction_;

 public:
  Store() : entities_(), counters_(sizeof...(Ts), 0), currentTransaction_() {}

  template <typename T>
  const Id create() {
    Id id = nextId<T>();
    entities_.emplace(id, T());

    if (currentTransaction_) {
      currentTransaction_->created.insert(id);
    }

    return id;
  }

  template <typename T, typename Fn>
  const Id create(Fn updater) {
    Id id = nextId<T>();
    auto [it, ok] = entities_.emplace(id, T());

    T* entity = &mapbox::util::get_unchecked<T>(it->second);
    updater(entity);

    if (currentTransaction_) {
      currentTransaction_->created.insert(id);
    }

    return id;
  }

  template <typename T>
  const Id lastId() const {
    uint32_t type = Entity::template which<T>();
    return {type + 1, counters_[type]};
  }

  constexpr bool isValid(const Id& id) const {
    return id.type > 0 && id.id > 0 && id.type <= sizeof...(Ts);
  }

  template <typename T>
  const T* find(Id id) const {
    if (!matchesType<T>(id)) {
      throw IdTypeMismatchError("in Store::find()");
    }

    auto it = entities_.find(id);
    if (it == entities_.end()) {
      return nullptr;
    }

    return &mapbox::util::get_unchecked<T>(it->second);
  }

  bool has(Id id) const { return entities_.find(id) != entities_.end(); }

  template <typename T>
  size_t count() const {
    size_t result = 0;
    for (auto it = entities_.begin(); it != entities_.end(); it++) {
      if (matchesType<T>(it->first)) {
        result++;
      }
    }
    return result;
  }

  size_t size() const { return entities_.size(); }

  template <typename T, typename Fn>
  void update(Id id, Fn updater) {
    if (!matchesType<T>(id)) {
      throw IdTypeMismatchError("in Store::update()");
    }

    auto it = entities_.find(id);
    if (it == entities_.end()) {
      throw MissingIdError("in Store::update()");
    }

    if (currentTransaction_) {
      auto& updated = currentTransaction_->updated;
      auto updatedIt = updated.find(it->first);
      if (updatedIt == updated.end()) {
        updated.insert(*it);
      }
    }

    T* entity = &mapbox::util::get_unchecked<T>(it->second);
    updater(entity);
  }

  void remove(Id id) {
    auto it = entities_.find(id);
    if (it == entities_.end()) {
      throw MissingIdError("in Store::remove()");
    }

    if (currentTransaction_) {
      auto node = entities_.extract(it);
      currentTransaction_->removed.insert(std::move(node));
    } else {
      entities_.erase(it);
    }
  }

  template <typename T>
  void clear() {
    for (auto it = entities_.begin(); it != entities_.end();) {
      if (matchesType<T>(it->first)) {
        if (currentTransaction_) {
          currentTransaction_->removed.insert(std::move(*it));
        }
        it = entities_.erase(it);
      } else {
        it++;
      }
    }
  }

  void clearAll() {
    if (currentTransaction_) {
      currentTransaction_->removed.insert(entities_.begin(), entities_.end());
    }
    entities_.clear();
  }

  template <typename Fn>
  bool transaction(Fn runner) {
    currentTransaction_ = std::make_unique<TransactionData>();
    bool success = true;
    try {
      runner();
    } catch (std::logic_error& e) {
      {
        auto& removed = currentTransaction_->removed;
        entities_.insert(removed.begin(), removed.end());
      }

      {
        auto& updated = currentTransaction_->updated;
        for (auto it = updated.begin(); it != updated.end(); it++) {
          entities_[it->first] = std::move(it->second);
        }
      }

      for (auto id : currentTransaction_->created) {
        entities_.erase(id);
      }

      success = false;
    }
    currentTransaction_.reset();
    return success;
  }

 private:
  template <typename T>
  bool matchesType(Id id) const {
    return Entity::template which<T>() + 1 == id.type;
  }

  template <typename T>
  Id nextId() {
    return nextId(Entity::template which<T>());
  }

  Id nextId(uint32_t type) { return {type + 1, nextCounter(type)}; }

  uint32_t nextCounter(uint32_t type) { return ++counters_[type]; }
};

}  // namespace chaoskit::state

#endif  // CHAOSKIT_STATE_STORE_H
