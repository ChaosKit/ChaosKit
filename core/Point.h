#ifndef CHAOSKIT_CORE_POINT_H
#define CHAOSKIT_CORE_POINT_H

namespace chaoskit::core {

class Point {
 public:
  Point() : x_(0), y_(0) {}
  Point(float x, float y) : x_(x), y_(y) {}
  Point(const Point &) = default;

  float x() const { return x_; }
  float y() const { return y_; }

  bool operator==(const Point &other) const {
    return x_ == other.x_ && y_ == other.y_;
  }

  Point operator+(const Point &other) const {
    return Point(x_ + other.x_, y_ + other.y_);
  }

  Point &operator+=(const Point &other) {
    x_ += other.x_;
    y_ += other.y_;
    return *this;
  }

 private:
  float x_, y_;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_POINT_H
