#ifndef CHAOSKIT_CORE_MODELS_MODELCOLLECTION_H
#define CHAOSKIT_CORE_MODELS_MODELCOLLECTION_H

#include <memory>
#include <vector>

namespace chaoskit::core {

template <typename Model>
class ModelCollection {
  using Collection = std::vector<std::shared_ptr<typename Model::Structure>>;
  Collection &dataRef_;

 public:
  template <typename T>
  class Iterator {
    typename Collection::iterator iterator_;
    std::unique_ptr<T> model_;

   public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using reference = T &;

    Iterator() = delete;
    Iterator(const Iterator<T> &it) : iterator_(it.iterator_), model_() {}
    Iterator(Iterator<T> &&it) noexcept : iterator_(it.iterator_), model_() {}
    explicit Iterator(typename Collection::iterator iterator)
        : iterator_(iterator), model_() {}

    Iterator<T> &operator=(const Iterator<T> &other) {
      iterator_ = other.iterator_;
      model_.release();
      return *this;
    }
    Iterator<T> &operator=(Iterator<T> &&other) noexcept {
      iterator_ = other.iterator_;
      model_.release();
      return *this;
    }

    Iterator<T> &operator++() {
      ++iterator_;
      model_.release();
      return *this;
    }
    const Iterator<T> operator++(int postfix) {
      Iterator<T> it = *this;
      iterator_++;
      model_.release();
      return it;
    }
    Iterator<T> &operator--() {
      --iterator_;
      model_.release();
      return *this;
    }
    const Iterator<T> operator--(int postfix) {
      Iterator<T> it = *this;
      iterator_--;
      model_.release();
      return it;
    }
    Iterator<T> &operator+=(difference_type diff) {
      iterator_ += diff;
      model_.release();
      return *this;
    }
    Iterator<T> &operator-=(difference_type diff) {
      iterator_ -= diff;
      model_.release();
      return *this;
    }
    Iterator<T> operator+(difference_type diff) {
      Iterator<T> it = *this;
      return (it += diff);
    }
    Iterator<T> operator-(difference_type diff) {
      Iterator<T> it = *this;
      return (it -= diff);
    }
    difference_type operator-(const Iterator<T> &other) {
      return iterator_ - other.iterator_;
    }
    reference operator[](difference_type n) { return *(*this + n); }

    reference operator*() {
      if (!model_) {
        model_ = std::make_unique<T>(*iterator_);
      }
      return *model_;
    }
    pointer operator->() {
      if (!model_) {
        model_ = std::make_unique<T>(*iterator_);
      }
      return model_.get();
    }
    bool operator==(const Iterator<T> &other) const {
      return iterator_ == other.iterator_;
    }
    bool operator!=(const Iterator<T> &other) const {
      return iterator_ != other.iterator_;
    }
    bool operator<(const Iterator<T> &other) const {
      return iterator_ < other.iterator_;
    }
    bool operator<=(const Iterator<T> &other) const {
      return iterator_ <= other.iterator_;
    }
    bool operator>(const Iterator<T> &other) const {
      return iterator_ > other.iterator_;
    }
    bool operator>=(const Iterator<T> &other) const {
      return iterator_ >= other.iterator_;
    }
  };

  using iterator = Iterator<Model>;
  using const_iterator = Iterator<const Model>;
  using size_type = typename Collection::size_type;

  ModelCollection() = delete;
  explicit ModelCollection(Collection &data) : dataRef_(data) {}

  size_type size() const { return dataRef_.size(); }

  iterator begin() { return iterator(dataRef_.begin()); }
  iterator end() { return iterator(dataRef_.end()); }
  const_iterator begin() const { return const_iterator(dataRef_.begin()); }
  const_iterator end() const { return const_iterator(dataRef_.end()); }
  const_iterator cbegin() const { return const_iterator(dataRef_.begin()); }
  const_iterator cend() const { return const_iterator(dataRef_.end()); }

  Model add() {
    return Model(
        dataRef_.emplace_back(std::make_shared<typename Model::Structure>()));
  }
  Model at(size_type index) const { return Model(dataRef_.at(index)); }
  Model operator[](size_type index) { return Model(dataRef_[index]); }
  void removeAt(size_type index) { dataRef_.erase(dataRef_.begin() + index); }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_MODELS_MODELCOLLECTION_H
