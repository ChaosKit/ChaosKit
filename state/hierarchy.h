#ifndef CHAOSKIT_STATE_HIERARCHY_H
#define CHAOSKIT_STATE_HIERARCHY_H

#include <type_traits>
#include <vector>

/**
 * Macro that overloads the Parent/ChildOf structures to resolve correctly all
 * the types for the specified relationship.
 */
#define DEFINE_RELATIONSHIP(ParentType, ChildType, ChildField) \
  namespace chaoskit::state {                                  \
  template <>                                                  \
  struct ParentOf<ChildType> {                                 \
    using Type = ParentType;                                   \
  };                                                           \
  template <>                                                  \
  struct ChildOf<ParentType> {                                 \
    using Type = ChildType;                                    \
    using FieldPtr = decltype(ChildField);                     \
    constexpr static FieldPtr field = ChildField;              \
  };                                                           \
  }

namespace chaoskit::state {

namespace detail {
template <typename>
struct IsVector : std::false_type {};
template <typename T, typename A>
struct IsVector<std::vector<T, A>> : std::true_type {};
}  // namespace detail

/** Structure that tells the type of the parent of a given type. */
template <typename C>
struct ParentOf {
  using Type = void;
};

/**
 * Structure that tells the type of a given type's children, as well as a
 * pointer to the field that contains them.
 */
template <typename P>
struct ChildOf {
  using Type = void;
  using FieldPtr = void*;
  constexpr static FieldPtr field = nullptr;
};

/** Checks if P is a parent of anything. */
template <typename P>
constexpr bool isParent() {
  return std::is_member_pointer_v<typename ChildOf<P>::FieldPtr>;
}

/** Checks if P is a child of anything. */
template <typename C>
constexpr bool isChild() {
  return !std::is_void_v<typename ParentOf<C>::Type>;
}

/** Checks if P is a parent of C. */
template <typename C, typename P>
constexpr bool isParentOf() {
  return std::is_same_v<typename ParentOf<C>::Type, P>;
}

/** Checks if C is a child of P. */
template <typename P, typename C>
constexpr bool isChildOf() {
  return std::is_same_v<typename ChildOf<P>::Type, C>;
}

/**
 * Checks if P and C are related, i.e. if P is the parent of C or vice versa.
 */
template <typename P, typename C>
constexpr bool areRelated() {
  return isParentOf<P, C>() || isChildOf<P, C>();
}

/**
 * Checks if P has a has-many relationship with C, i.e. if P's children are
 * stored in a vector.
 */
template <typename P>
constexpr bool hasManyChildren() {
  static_assert(isParent<P>());
  return detail::IsVector<
      std::remove_reference_t<decltype(P().*(ChildOf<P>::field))>>::value;
}

/**
 * Checks if P has a has-one relationship with C, i.e. if P's child is stored
 * in a pointer.
 */
template <typename P>
constexpr bool hasOneChild() {
  static_assert(isParent<P>());
  // TODO: check for a pointer
  return !detail::IsVector<
      std::remove_reference_t<decltype(P().*(ChildOf<P>::field))>>::value;
}

/**
 * Returns a reference to the field containing the children of the parent.
 *
 * It's either a reference to a vector or a reference to a pointer, depending on
 * whether P haa a has-many or has-one relationship.
 */
template <typename P>
decltype(auto) getChildren(P& parent) {
  static_assert(isParent<P>());
  return parent.*(ChildOf<P>::field);
}

/**
 * Associates a child with a parent.
 *
 * In case of a has-one relationship, this sets the child pointer to the given
 * child. In case of a has-many relationship, this adds the child to the vector
 * of children.
 */
template <typename P, typename C>
void associateChild(P& parent, C* child) {
  static_assert(isParentOf<C, P>());
  if constexpr (hasManyChildren<P>()) {
    getChildren(parent).push_back(child);
  } else {
    getChildren(parent) = child;
  }
}

/**
 * Dissociates a child with a parent.
 *
 * In case of a has-one relationship, this sets the child pointer to null. In
 * case of a has-many relationship, this removes the child pointer from the
 * vector of children.
 */
template <typename P, typename C>
void dissociateChild(P& parent, C* child) {
  static_assert(isParentOf<C, P>());
  if constexpr (hasManyChildren<P>()) {
    auto& children = getChildren(parent);
    children.erase(std::remove(std::begin(children), std::end(children), child),
                   std::end(children));
  } else {
    auto& children = getChildren(parent);
    if (children == child) {
      children = nullptr;
    }
  }
}

}  // namespace chaoskit::state

#endif  // CHAOSKIT_STATE_HIERARCHY_H
